#include <PostProcessingHumanSegmentationLayer.hpp>
#include <boost/type_index.hpp>
#include <iostream>

const std::string PostProcessingHumanSegmentationLayer::HumanSegmentationModelPath = "../../../../StreamingApp/resources/human_segmentation_pphumanseg_2023mar.onnx";
const std::string PostProcessingHumanSegmentationLayer::BackgroundPath = "../../../../StreamingApp/resources/backgrounds/background.png";

PostProcessingHumanSegmentationLayer::PostProcessingHumanSegmentationLayer()
{
	std::cout << boost::typeindex::type_id<PostProcessingHumanSegmentationLayer>().pretty_name() << " layer instance is created" << std::endl;

	HumanSegmentationModel = cv::dnn::readNet(HumanSegmentationModelPath);
	if (HumanSegmentationModel.empty())
	{
        std::cerr << "Error: HumanSegmentationModel was not loaded." << std::endl;
    }

    BackgroundImage = cv::imread(BackgroundPath);

	HumanSegmentationModel.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
	HumanSegmentationModel.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
}

PostProcessingHumanSegmentationLayer::~PostProcessingHumanSegmentationLayer()
{
	std::cout << boost::typeindex::type_id<PostProcessingHumanSegmentationLayer>().pretty_name() << " layer instance is destroyed" << std::endl;
}

void PostProcessingHumanSegmentationLayer::PostProcessFrame(cv::Mat& InFrame)
{
    processFrame(InFrame);
}

void PostProcessingHumanSegmentationLayer::extractMask(const cv::Mat& srcFrame, cv::Mat& outMask)
{    
    cv::Mat inputBlob = cv::dnn::blobFromImage(srcFrame, 1.0 / 127.5, InputSize, cv::Scalar(127.5, 127.5, 127.5), true, false);

    HumanSegmentationModel.setInput(inputBlob, "x");
    cv::Mat OutputBlob = HumanSegmentationModel.forward("save_infer_model/scale_0.tmp_1");

    int H = OutputBlob.size[2];
    int W = OutputBlob.size[3];
    cv::Mat BgScores(H, W, CV_32FC1, OutputBlob.ptr<float>(0, 0));
    cv::Mat HumanScores(H, W, CV_32FC1, OutputBlob.ptr<float>(0, 1));

    cv::Mat CurrentScores;
    cv::resize(HumanScores - BgScores, CurrentScores, srcFrame.size(), 0, 0, cv::INTER_CUBIC);

    if (HistoryScores.empty() || HistoryScores.size() != CurrentScores.size())
    {
        CurrentScores.copyTo(HistoryScores);
    }
    else
    {
        cv::addWeighted(HistoryScores, 0.75, CurrentScores, 0.25, 0, HistoryScores);
    }

    cv::Mat BinaryMask;
    cv::threshold(HistoryScores, BinaryMask, 0.85, 255.0, cv::THRESH_BINARY);
    BinaryMask.convertTo(BinaryMask, CV_8U);

    cv::Mat EdgePreservedMask;
    cv::bilateralFilter(BinaryMask, EdgePreservedMask, 9, 75, 75);

    cv::Mat CleanMask;
    cv::Mat Kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(2, 2));
    cv::morphologyEx(EdgePreservedMask, CleanMask, cv::MORPH_OPEN, Kernel);

    std::vector<std::vector<cv::Point>> Contours;
    cv::findContours(CleanMask, Contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    outMask = cv::Mat::zeros(CleanMask.size(), CV_8UC1);
    double minAreaThreshold = (srcFrame.cols * srcFrame.rows) * 0.005;

    for (size_t i = 0; i < Contours.size(); i++)
    {
        if (cv::contourArea(Contours[i]) > minAreaThreshold)
        {
            cv::drawContours(outMask, Contours, static_cast<int>(i), cv::Scalar(255), cv::FILLED);
        }
    }
}

void PostProcessingHumanSegmentationLayer::processFrame(cv::Mat& ioFrame) {
    
    if (BackgroundImage.size() != ioFrame.size())
    {
        cv::resize(BackgroundImage, BackgroundImage, ioFrame.size());
    }
    
    cv::Mat HumanMask;
    extractMask(ioFrame, HumanMask);

    cv::Mat BackgroundMask;
    cv::bitwise_not(HumanMask, BackgroundMask);
    BackgroundImage.copyTo(ioFrame, BackgroundMask);
}
