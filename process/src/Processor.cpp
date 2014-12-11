#include "Processor.h"

using namespace cv;

Processor::Processor(std::string imgName){
    image = imread(imgName, CV_LOAD_IMAGE_COLOR);

    if(!image.data){
        std::cout << "No image data :(" << std::endl;
        return;
    }
}

Processor::~Processor()
{}

void Processor::blurImage(double kernelwidth, double kernelheight){
    Mat blurimg = image.clone();
    //for (int i = 1; i < max_kernel_length; i += 2){
        // Size(w,h): size of the kernel to be used, with w, h odd and positive
        // stdevx, stddevy... if pass in 0, calculated from kernel size
        GaussianBlur(image, blurimg, Size(kernelwidth, kernelheight), 0, 0); 
    //}    
    image = blurimg;

}

void Processor::doSobel(int kernelsize){
    Mat grayimg;
    cvtColor(image, grayimg, CV_RGB2GRAY);
    Mat gradX, gradY;
    Mat absgradX, absgradY;
    Mat grad;
    // Sobel(src, dst, int ddepth, int dx, int dy, int ksize, double scale, double delta, int borderType)
    // ddepth: output image depth, same as source if set to -1
    // xorder: order of derivative x
    // yorder: order of derivative y
    // ksize: size of extended Sobel kernel
    // scale: optional scaling of output
    // delta: optional value to add to results
    // borderType: extrapolation method at borders
    Sobel(grayimg, gradX, CV_32F, 1, 0, kernelsize, 1.0, 0, BORDER_DEFAULT);
    //convertScaleAbs(gradX, absgradX);
    //absgradX = abs(gradX);
    Sobel(grayimg, gradY, CV_32F, 0, 1, kernelsize, 1.0, 0, BORDER_DEFAULT);
    //convertScaleAbs(gradY, absgradY);
   // absgradY = abs(gradY);

    
    // Note that it matters whether you're dealing with bytes (gray) or ints
    std::cout << (absgradX.size == absgradY.size) << std::endl; 
    std::cout << gradX.depth() << std::endl;
    std::cout << gradY.depth() << std::endl;
    std::cout << absgradX.depth() << std::endl;
    std::cout << absgradY.depth() << std::endl;



//    magnitude(absgradX, absgradY, grad);
    magnitude(gradX, gradY, grad);

    namedWindow("gradWind", CV_WINDOW_AUTOSIZE);
    imshow("gradWind", grad);
    waitKey(0);


}


void Processor::display(std::string windowName){
    namedWindow(windowName, WINDOW_NORMAL);
    imshow(windowName, image);

    waitKey(0);
    return;
}


void Processor::placeStrokes(){
    // create binary matrix of size image
    // actually... technically... of size image * scale... :(    
    // brush width, wr. etc. all in terms of canvas size... 

    int k = 5000; // try k random points
    int stopthresh = 100; // if failed to place stopthresh times, stop algorithm 
    double scale = 1;
    double wr = 10; 
    int countstop = 0;

    Size imgSize = image.size();
    Scalar s = 0;
    Mat maskimg(imgSize.height*2, imgSize.width*2, CV_8UC1, s);
    Size maskSize = maskimg.size();

    std::vector<Brushstroke> strokes;
    srand(time(NULL));

    std::cout << countNonZero(maskimg) << std::endl;

    for (int i = 0; i < k; ++i){
        if(countstop > stopthresh){
            std::cout << "breaking: reached stopthresh" << std::endl;
            break;
        }
        std::cout << countstop << std::endl;
        int r = rand()%maskSize.height;
        int c = rand()%maskSize.width;                  
       
        int rs = std::max(0, (int)(r-wr/2.0)); 
        int re = std::min((int)(r+wr/2.0), maskSize.height);
        int cs = std::max(0, (int)(c-wr/2.0));
        int ce = std::min((int)(c+wr/2.0), maskSize.width);

        //std::cout << "trying " << r << ", " << c << std::endl;
        if (countNonZero(maskimg(Range(rs, re), Range(cs, ce))) >= 1){
            //std::cout << "rejected " << std::endl;
            countstop++;
            continue;
        }
        countstop = 0;
        //std::cout << "setting " << std::endl;
        maskimg.at<uchar>(r,c) = 255; 
        
        //std::cout << "r: " << rs << ", " << re << std::endl;
        //std::cout << "c: " << cs << ", " << ce << std::endl;


        //maskimg.at<uchar>(rs, cs) = 255;
        //maskimg.at<uchar>(rs, ce) = 255;
        //maskimg.at<uchar>(re, cs) = 255;
        //maskimg.at<uchar>(re, ce) = 255;

        //Vec3b col = image.at<Vec3b>(Point(x/2,y/2));
    
    }
  
    std::cout << countNonZero(maskimg) << std::endl;

    namedWindow("gradWind", CV_WINDOW_NORMAL);
    imshow("gradWind", maskimg);
    waitKey(0); 

    std::cout << "postprocessing... fill in the holes" << std::endl;
    
    for(int r = 0; r < maskSize.height; ++r){
        for(int c = 0; c < maskSize.width; ++c){
       
            int rs = std::max(0, (int)(r-wr/2.0)); 
            int re = std::min((int)(r+wr/2.0), maskSize.height);
            int cs = std::max(0, (int)(c-wr/2.0));
            int ce = std::min((int)(c+wr/2.0), maskSize.width);

            //std::cout << "trying " << r << ", " << c << std::endl;
            if (countNonZero(maskimg(Range(rs, re), Range(cs, ce))) >= 1){
                //std::cout << "rejected " << std::endl;
                continue;
            }
            std::cout << "setting " << r << ", " << c << std::endl;
            maskimg.at<uchar>(r,c) = 255; 
        }
    }

    namedWindow("pWind", CV_WINDOW_NORMAL);
    imshow("pWind", maskimg);
    waitKey(0); 

   



    for (int i = 0; i < imgSize.height; ++i){
        for(int j = 0; j < imgSize.width; ++j){
    
            if(maskimg.at<uchar>(i*2.0, j*2.0) > 100){
                image.at<Vec3b>(i,j) = {0, 0, 0};
            }

        }
    }

//    display();


/*
    for (int i = 0; i < imgSize.height; ++i){
        for(int j = 0; j < imgSize.width; ++j){

            Vec3b col = image.at<Vec3b>(Point(j,i));
                        
            std::cout << (int)col[0] << ", " << (int)col[1] << ", " << (int)col[2] << std::endl;


        }
    }  
*/

}



