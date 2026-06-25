#include <PostProcessingFaceDetectionLayer.hpp>
#include <boost/type_index.hpp>
#include <iostream>

const std::string PostProcessingFaceDetectionLayer::FaceCascadePath = "../../../../StreamingApp/resources/models/face_detection_yunet_2023mar.onnx";

PostProcessingFaceDetectionLayer::PostProcessingFaceDetectionLayer()
{
	std::cout << boost::typeindex::type_id<PostProcessingFaceDetectionLayer>().pretty_name() << " layer instance is created" << std::endl;

	cv::Size InputSize(640, 480);
	FaceDetector = cv::FaceDetectorYN::create(FaceCascadePath, "", InputSize, 0.6f, 0.3f, 5000, cv::dnn::DNN_BACKEND_CUDA, cv::dnn::DNN_TARGET_CUDA);

	if (FaceDetector.empty())
	{
		std::cerr << "Error: Face detection model was not loaded." << std::endl;
	}

	/*
		face regognition will always work in the background
	*/
	ActivateLayer();
}

PostProcessingFaceDetectionLayer::~PostProcessingFaceDetectionLayer()
{
	std::cout << boost::typeindex::type_id<PostProcessingFaceDetectionLayer>().pretty_name() << " layer instance is destroyed" << std::endl;
}

void PostProcessingFaceDetectionLayer::PostProcessFrame(cv::Mat& InFrame)
{
	FacesRoi.clear();

	/*
		Detect face
	*/
	cv::Mat Faces;
	FaceDetector->detect(InFrame, Faces);

	for (int i = 0; i < Faces.rows; ++i)
	{
		int x = static_cast<int>(Faces.at<float>(i, 0));
		int y = static_cast<int>(Faces.at<float>(i, 1));
		int w = static_cast<int>(Faces.at<float>(i, 2));
		int h = static_cast<int>(Faces.at<float>(i, 3));

		/*
			Blur the face
		*/
		if (bDebugDrawEnabled)
		{
			cv::rectangle(InFrame, cv::Rect(x, y, w, h), cv::Scalar(0, 255, 0), 2);
		}

		cv::Rect FaceRect = cv::Rect(x, y, w, h) & cv::Rect(0, 0, InFrame.cols, InFrame.rows);
		cv::Mat FaceROI = InFrame(FaceRect);
		FacesRoi.push_back(FaceROI);

		/*
			Eyes detection
		*/
		int RightEyeX = static_cast<int>(Faces.at<float>(i, 4));
		int RightEyeY = static_cast<int>(Faces.at<float>(i, 5));
		int LeftEyeX = static_cast<int>(Faces.at<float>(i, 6));
		int LeftEyeY = static_cast<int>(Faces.at<float>(i, 7));

		if (bDebugDrawEnabled)
		{
			cv::circle(InFrame, cv::Point(RightEyeX, RightEyeY), 20, cv::Scalar(255, 0, 0), -1);
			cv::circle(InFrame, cv::Point(LeftEyeX, LeftEyeY), 20, cv::Scalar(255, 0, 0), -1);
		}
	}
}

const std::vector<cv::Mat>& PostProcessingFaceDetectionLayer::GetFacesRoi() const
{
	return FacesRoi;
}
