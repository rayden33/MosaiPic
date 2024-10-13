#include<iostream>
#include<filesystem>
#include<opencv2/opencv.hpp>
#include<opencv2/highgui.hpp>

using namespace std;
using namespace cv;

/**
 * Load image by path
 * @param filename Image filename with full path
 * @return cv::Mat object 
 */
Mat loadImage(const string& filename)
{
    Mat img = imread(filename, IMREAD_COLOR);

    if(img.empty())
    {
        cerr << "Could not read the image: " << filename;
        return Mat();
    }

    return img;
}

/**
 * Split the main image into blocks
 * @param image Main image
 * @param blockWidth Width of one block
 * @param blockHeight Height of one block
 * @return Vector list of main image blocks
 */
vector<Mat> splitImageIntoBlocks(const Mat& image, int blockWidth, int blockHeight)
{
    vector<Mat> blocks;

    for(int y=0; y<image.rows; y+=blockHeight)
    {
        for(int x=0; x<image.cols; x+=blockWidth)
        {
            if(x+blockWidth <= image.cols && y+blockHeight <= image.rows)
            {
                Rect region(x,y,blockWidth,blockHeight);
                blocks.push_back(image(region));
            }
        }
    }  

    return blocks;  
}

int main()
{

}