#include <FramePostProcessingHandler.hpp>
#include <boost/type_index.hpp>
#include <iostream>

FramePostProcessingHandler::FramePostProcessingHandler()
{
	std::cout << boost::typeindex::type_id<FramePostProcessingHandler>().pretty_name() << " pipeline instance is created" << std::endl;
}

FramePostProcessingHandler::~FramePostProcessingHandler()
{
	std::cout << boost::typeindex::type_id<FramePostProcessingHandler>().pretty_name() << " pipeline instance is destroyed" << std::endl;
}

void FramePostProcessingHandler::PostProcessFrame(cv::Mat& InFrame)
{
	/*
		Flip the frame
	*/
	cv::flip(InFrame, InFrame, 1);

	/*
		Face detection part
	*/
	std::string cascade_path = "../../../../StreamingApp/resources/haarcascades/haarcascade_frontalface_default.xml";
	std::string eye_path = "../../../../StreamingApp/resources/haarcascades/haarcascade_eye.xml";
	std::string smile_path = "../../../../StreamingApp/resources/haarcascades/haarcascade_smile.xml";

	cv::CascadeClassifier faceCascade;
	if (!faceCascade.load(cascade_path))
	{
		std::cerr << "Error: Could not load cascade classifier XML file." << std::endl;
		return;
	}

	cv::CascadeClassifier eyeCascade;
	if (!eyeCascade.load(eye_path))
	{
		std::cerr << "Error: Could not load cascade classifier XML file." << std::endl;
		return;
	}

	cv::CascadeClassifier smileCascade;
	if (!smileCascade.load(smile_path))
	{
		std::cerr << "Error: Could not load cascade classifier XML file." << std::endl;
		return;
	}

	std::vector<cv::Rect> faces;
	std::vector<cv::Rect> eyes;
	std::vector<cv::Rect> smiles;

	faceCascade.detectMultiScale(InFrame, faces, 1.1, 4, 0, cv::Size(20, 20));

	cv::Mat BluredFrame;
	cv::GaussianBlur(InFrame, BluredFrame, cv::Size(51, 51), 0);


	/*
		adding background and me there in small square
	*/
	//cv::Mat background = cv::imread("../../../../StreamingApp/resources/backgrounds/background.png");

	//cv::Mat ResizedBackground;
	//cv::resize(background, ResizedBackground, Frame.size());

	//cv::Mat FrameCopy = Frame.clone();

	//ResizedBackground.copyTo(Frame);

	for (const auto& face : faces)
	{
		cv::rectangle(InFrame, face, cv::Scalar(0, 255, 0), 3);

		smileCascade.detectMultiScale(InFrame(face), smiles, 1.8, 22);

		for (const auto& smile : smiles)
		{
			cv::rectangle(InFrame(face), smile, cv::Scalar(0, 0, 255), 3);
		}

		eyeCascade.detectMultiScale(InFrame(face), eyes, 1.1, 22);

		for (const auto& eye : eyes)
		{
			cv::rectangle(InFrame(face), eye, cv::Scalar(255, 0, 0), 3);


			/*
				1. Blur only face
			*/
			//cv::Mat faceROI = FrameCopy(face);
			//cv::GaussianBlur(faceROI, faceROI, cv::Size(51, 51), 0);

			/*
				2. Blur everything except face
			*/
			//cv::Mat faceROI = Frame(face);
			//cv::Mat targetROI = BluredFrame(face);
			//faceROI.copyTo(targetROI);
			//BluredFrame.copyTo(Frame);
			//faceROI.copyTo(Frame);

			/*
				put my face on top of background
			*/
			//cv::Mat faceROI = FrameCopy(face);
			//cv::Mat targetROI = Frame(face);
			//faceROI.copyTo(targetROI);
		}

		/*
			1. Blur only face
		*/
		//cv::Mat faceROI = FrameCopy(face);
		//cv::GaussianBlur(faceROI, faceROI, cv::Size(51, 51), 0);

		/*
			2. Blur everything except face
		*/
		//cv::Mat faceROI = Frame(face);
		//cv::Mat targetROI = BluredFrame(face);
		//faceROI.copyTo(targetROI);
		//BluredFrame.copyTo(Frame);
		//faceROI.copyTo(Frame);

		/*
			put my face on top of background
		*/
		//cv::Mat faceROI = FrameCopy(face);
		//cv::Mat targetROI = Frame(face);
		//faceROI.copyTo(targetROI);
	}

	cv::putText(InFrame,
		"WebRTC Stream Live",
		cv::Point(10, 15),
		cv::FONT_HERSHEY_SIMPLEX,
		0.6,
		cv::Scalar(0, 255, 0),
		1,
		cv::FILLED);

	// Apply a Bilateral Filter to remove noise while keeping edges sharp
	//cv::bilateralFilter(Frame, ProcessedFrame, 9, 75, 75);

	// Convert to a more accurate color space if streaming over networks
	//cv::cvtColor(ProcessedFrame, ProcessedFrame, cv::COLOR_BGR2YCrCb);

	//cv::fastNlMeansDenoisingColored(Frame, Frame);

	//Frame = ProcessedFrame;
}