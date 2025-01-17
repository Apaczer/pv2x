#include "scaledimage.h"

ScaledImage::ScaledImage(std::string filename, int rotateMode) {
	this->image=NULL;
	this->scaledImage=NULL;
	
	this->init();
	
	// convert image so screen format
	SDL_Surface *tmpload=IMG_Load(filename.c_str());
	if (tmpload==NULL) {
		this->loadError=1;
		return;
	}
	
	SDL_Surface *tmp=sdlCreateSurface(tmpload->w,tmpload->h);
	sdlLock(tmp);
	sdlLock(tmpload);
	SDL_BlitSurface(tmpload,NULL,tmp,NULL);
	sdlUnlock(tmpload);
	SDL_FreeSurface(tmpload);
	
	// rotate if necessary
	int rotate=0;
	
	switch (rotateMode) {
		case ROTATEMODE_ROTATE:
			rotate=1;
			this->image=sdlCreateSurface(tmp->h,tmp->w);
			break;
		case ROTATEMODE_BEST:
			if (tmp->h>tmp->w) {
				this->image=sdlCreateSurface(tmp->h,tmp->w);
				rotate=1;
			} else {
				this->image=sdlCreateSurface(tmp->w,tmp->h);
			}
			break;
		case ROTATEMODE_FLIPH:
			rotate=2;
			this->image=sdlCreateSurface(tmp->w,tmp->h);
			break;
		case ROTATEMODE_FLIPV:
			rotate=3;
			this->image=sdlCreateSurface(tmp->w,tmp->h);
			break;
		default:
			this->image=sdlCreateSurface(tmp->w,tmp->h);
			break;
	}
	
	sdlLock(this->image);
	if (rotate) {
		Uint16 *src=(Uint16*)tmp->pixels;
		Uint16 *dst=(Uint16*)this->image->pixels;
		int srcpitch=tmp->pitch>>1;
		int dstpitch=image->pitch>>1;

		if (rotate==1) {
			for (int y=0;y<tmp->h;y++) {
				for (int x=0;x<tmp->w;x++) {
					if (y*srcpitch>=x+1) {
						if (y!=0 && x!=0) dst[x*dstpitch+y-1]=src[y*srcpitch-x-1];
						else dst[x*dstpitch+y-1]=src[y*srcpitch+x-1];
					}
				}
			}
		} else if (rotate==2) {
			for (int y=0;y<tmp->h;y++) {
				for (int x=0;x<tmp->w;x++) {
					if (y*srcpitch>=x+1) {
						dst[y*dstpitch+x] = src[y*srcpitch-1-x];
					}
				}
			}
		} else if (rotate==3) {
			for (int y=0;y<tmp->h;y++) {
				if (tmp->h>=y+1)
				memcpy(dst+y*dstpitch, src+(tmp->h-1-y)*srcpitch, srcpitch*2);
			}
		}
	} else {
		SDL_BlitSurface(tmp,NULL,this->image,NULL);
	}
	sdlUnlock(tmp);
	SDL_FreeSurface(tmp);
	sdlUnlock(this->image);
}

ScaledImage::~ScaledImage() {
	SDL_FreeSurface(this->image);
	this->image=NULL;
	if (this->isScaled) {
		SDL_FreeSurface(this->scaledImage);
		this->scaledImage=NULL;
	}
}

void ScaledImage::init() {
	this->image=NULL;
	this->scaledImage=NULL;
	this->isScaled=0;
	this->loadError=0;
}

void ScaledImage::scale(double factor, double angle) {
	if (this->isScaled) {
		SDL_FreeSurface(this->scaledImage);
		this->scaledImage=NULL;
	}
	
	this->scaledImage=sdlCreateSurface((int)(this->image->w*65536.0*factor)>>16, (int)(this->image->h*65536.0*factor)>>16,1);
	this->isScaled=1;

	sdlLock(this->scaledImage);
	sdlLock(this->image);
	
	Uint16 *src=(Uint16*)this->image->pixels;
	Uint16 *dst=(Uint16*)this->scaledImage->pixels;
	
	int sinfp=(int)((sin(angle)*65536.0)/factor);
        int cosfp=(int)((cos(angle)*65536.0)/factor);

        int xc=(this->image->w<<15) - ((this->scaledImage->w>>1)*(cosfp+sinfp));
        int yc=(this->image->h<<15) - ((this->scaledImage->h>>1)*(cosfp-sinfp));

        int tx,ty;
        int x,y;
        int tempx,tempy;
        int lasttempx=0, lasttempy=0;
        int colorcount;
        int ipx,ipy;
        Uint8 r,g,b;
        int rsum=0,gsum=0,bsum=0;
        Uint32 newpixel;
        int scaledpitch=this->scaledImage->pitch>>1;
	int imagepitch=this->image->pitch>>1;

        for ( y=0; y<this->scaledImage->h; y++ ) {

                tx=xc;
                ty=yc;

                for( x=0; x<this->scaledImage->w; x++ ) {

                        tempx=(tx>>16);
                        tempy=(ty>>16);

                        if( (tempx<0) || (tempx>=this->image->w) || (tempy<0) || (tempy>=this->image->h) ) {
                                *dst = 0;
                        } else {
                                if (
					(tempx-lasttempx>0) ||
					(tempy-lasttempy>0)
				) {
					rsum=0;
					gsum=0;
					bsum=0;
	                                colorcount=0;
	                                ipy=0;
	                                for (ipy=0;ipy<(tempy-lasttempy)+1;ipy++) {
						for (ipx=0;ipx<(tempx-lasttempx)+1;ipx++) {
							SDL_GetRGB(*(src+tempx-ipx+((tempy-ipy)*imagepitch)),this->image->format,&r,&g,&b);
							rsum+=r;
							gsum+=g;
							bsum+=b;
							colorcount++;
						}
					}
					newpixel=SDL_MapRGB(
						this->image->format,
						(int)rsum/(colorcount),
						(int)gsum/(colorcount),
						(int)bsum/(colorcount)
					);
					*(dst+x+y*scaledpitch)=(Uint16)newpixel;
				} else {
					*(dst+x+y*scaledpitch) = *(src+tempx+tempy*imagepitch);
				}
                        }

                        tx+=cosfp;
                        ty-=sinfp;

                        lasttempx=tempx;
                        lasttempy=tempy;
                }
                lasttempx=0;
                xc+=sinfp;
                yc+=cosfp;
        }

	sdlUnlock(this->image);
	sdlUnlock(this->scaledImage);
}

void ScaledImage::blit(SDL_Surface *target) {
	this->blit(target,NULL);
}

void ScaledImage::blit(SDL_Surface *target, int x, int y) {
	SDL_Rect dest;
	dest.x=x;
	dest.y=y;
	this->blit(target,NULL,&dest);
}

void ScaledImage::blit(SDL_Surface *target, SDL_Rect *rect) {
	this->blit(target,rect,NULL);
}

void ScaledImage::blit(SDL_Surface *target, SDL_Rect *rect, SDL_Rect *destrect) {
	sdlLock(target);
	if (this->isScaled) {
		sdlLock(this->scaledImage);
		SDL_BlitSurface(this->scaledImage, rect, target, destrect);
		sdlUnlock(this->scaledImage);
	} else {
		sdlLock(this->image);
		SDL_BlitSurface(this->image, rect, target, destrect);
		sdlUnlock(this->image);
	}
	sdlUnlock(target);
}
