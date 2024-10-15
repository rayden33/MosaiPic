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
    string mainImagePath;
    string smallImagesDirPath;
    string resultImagePath;
    int mainImageScale;
    int mosaicBlockWidth;
    int mosaicBlockHeight;
    int progressBarWidth;
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

    config.mainImagePath = ini.GetValue("settings","main_image_path", "main.jpg");
    config.smallImagesDirPath = ini.GetValue("settings","small_images_dir_path", "parts");
    config.resultImagePath = ini.GetValue("settings","result_image_path", "result.jpg");
    config.mainImageScale = stoi(ini.GetValue("sizes", "mosaic_size_scale", "1"));
    config.mosaicBlockWidth = stoi(ini.GetValue("sizes", "mosaic_block_width", "32"));
    config.mosaicBlockHeight = stoi(ini.GetValue("sizes", "mosaic_block_height", "32"));
    config.progressBarWidth = stoi(ini.GetValue("sizes", "progress_bar_width", "50"));
    
    return true;
}

int countFilesInDirectory(const std::string& path) {
    int fileCount = 0;

    if (!filesystem::exists(path) || !filesystem::is_directory(path)) {
        std::cerr << "The path does not exist or is not a folder: " << path << std::endl;
        return -1;
    }

    for (const auto& entry : filesystem::directory_iterator(path)) {
        if (filesystem::is_regular_file(entry.status())) {
            fileCount++;
        }
    }
    return fileCount;
}

void showProgress(int progress, int total, int width, const string& detail = "")
{
    int pos = width * progress / total;
    const char spinner[] = {'|', '/', '-', '\\'};

    cout << "[";
    for(int i=0; i<width; i++)
    {
        if(i < pos) cout << "=";
        else if (i == pos) cout << ">";
        else cout << " ";
    }
    cout << "] " << int(progress * 100.0 / total) << "% ";
    std::cout << spinner[progress % 4] << " " << detail << "\r";
    cout.flush();
}


int main()
{
    Config config;
    if(!loadIniConfiguration("config.ini", config))
        return 1;

    string mainImagePath = config.mainImagePath;
    string smallImagesDirPath = config.smallImagesDirPath;
    string resultImagePath = config.resultImagePath;

    int smallImagesCount = countFilesInDirectory(config.smallImagesDirPath);
    int mainImageScale = config.mainImageScale;
    int blockWidth = config.mosaicBlockWidth;
    int blockHeight = config.mosaicBlockHeight;
    int progressBarWidth = config.progressBarWidth;
    int counter = 0;
    int numCols;
    int bestMatchIndex;
    int row;
    int col;
    int curWidth;
    int curHeight;

    vector<Mat> smallImages;
    vector<Scalar> averageColors;
    vector<Mat> mosaicBlocks;

    Scalar avgColor;

    Mat mainImage = loadImage(mainImagePath);
    Mat smallImage;
    Mat resultImage;

    if(mainImage.empty())
    {
        cerr<<"Could not open the main image: "<< mainImagePath;
        return -1;
    }

    resize(mainImage, mainImage, Size(mainImage.cols * mainImageScale, mainImage.rows * mainImageScale), INTER_LINEAR);
    mosaicBlocks = splitImageIntoBlocks(mainImage, blockWidth, blockHeight);
    resultImage = Mat(mainImage.size(), mainImage.type());

    for(const auto& entry : filesystem::directory_iterator(smallImagesDirPath))
    {
        smallImage = loadImage(entry.path().string());
        if(!smallImage.empty())
        {
            resize(smallImage, smallImage, Size(blockWidth, blockHeight));
            smallImages.push_back(smallImage);
            averageColors.push_back(computeAverageColor(smallImage));
            showProgress(counter++, smallImagesCount, progressBarWidth, entry.path().filename().string());
        }
    }

    numCols = mainImage.cols / blockWidth;

    for(int i=0; i<mosaicBlocks.size(); i++)
    {
        avgColor = computeAverageColor(mosaicBlocks[i]);
        bestMatchIndex = findBestMatch(avgColor, averageColors);

        row = (i/numCols) * blockHeight;
        col = (i%numCols) * blockWidth;

        if(bestMatchIndex < 0)
        {
            cerr<<"Not found best index";
            return -1;
        }

        curWidth = min(blockWidth, mainImage.cols - col);
        curHeight = min(blockHeight, mainImage.rows - row);

        if(col + blockWidth <= resultImage.cols && row + blockHeight <= resultImage.rows)
        {
            smallImages[bestMatchIndex].copyTo(resultImage(Rect(col, row, curWidth, curHeight)));
        }
    }

    imwrite(resultImagePath, resultImage);
    cout<< "Photomosaic created";

    return 0;
}