#define NOMINMAX
#include<iostream>
#include<filesystem>
#include<opencv2/opencv.hpp>
#include<opencv2/highgui.hpp>
#include<SimpleIni.h>


using namespace std;
using namespace cv;

struct Config
{
    int mainImageScale;
    int mosaicBlockWidth;
    int mosaicBlockHeight; 
};

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
        cerr << "Could not read the image: [" << filename << "]\n";
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

/**
 * Find the closest average color from the list of average mosaic colors for a given average color
 * @param avgColor The given average color
 * @param avgColors The list of average mosaic colors
 * @return Index of the closest average color from the list of average colors of the mosaic (or -1 if not found closest average color)
 */
int findBestMatch(const Scalar& avgColor, const vector<Scalar>& avgColors)
{
    double minDist = std::numeric_limits<double>::max();
    int bestIndex = -1;
    for(int i=0; i<avgColors.size(); i++)
    {
        double dist = norm(avgColor - avgColors[i]);
        if(dist < minDist)
        {
            minDist = dist;
            bestIndex = i;
        }
    }

    return bestIndex;
}

/**
 * Calculate average color
 * @param block The given image
 * @return Average color of the given image
 */
Scalar computeAverageColor(const Mat& block)
{
    Scalar avgColor = mean(block);
    return avgColor;
}

/**
 * Load ini config file and fill config struct
 * @param filename ini config filename with full path
 * @param config Config struct for storage config values
 * @return Whether the function was processed successfully or not (true - success; false - fail)
 */
bool loadIniConfiguration(const string& filename, Config& config)
{
    CSimpleIniA ini;
    ini.SetUnicode();

    SI_Error rc = ini.LoadFile(filename.c_str());
    if(rc < 0)
    {
        cerr << "Could not read config file: [" << filename <<"]\n";
        return false;
    }

    config.mainImageScale = stoi(ini.GetValue("settings", "mosaic_size_scale", "1"));
    config.mosaicBlockWidth = stoi(ini.GetValue("settings", "mosaic_block_width", "32"));
    config.mosaicBlockHeight = stoi(ini.GetValue("settings", "mosaic_block_height", "32"));
}


int main()
{
    Config config;
    if(!loadIniConfiguration("config.ini", config))
        return 1;

    
}