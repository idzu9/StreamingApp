#include <PostProcessingFaceDetectionLayer.hpp>
#include <boost/type_index.hpp>
#include <iostream>

const std::string PostProcessingFaceDetectionLayer::FaceCascadePath = "../../../../StreamingApp/resources/haarcascades/haarcascade_frontalface_default.xml";
const std::string PostProcessingFaceDetectionLayer::EyePath = "../../../../StreamingApp/resources/haarcascades/haarcascade_eye.xml";
const std::string PostProcessingFaceDetectionLayer::SmilePath = "../../../../StreamingApp/resources/haarcascades/haarcascade_smile.xml";

PostProcessingFaceDetectionLayer::PostProcessingFaceDetectionLayer()
{
	std::cout << boost::typeindex::type_id<PostProcessingFaceDetectionLayer>().pretty_name() << " layer instance is created" << std::endl;

	if (!FaceCascade.load(FaceCascadePath))
	{
		std::cerr << "Error: Could not load cascade classifier XML file." << std::endl;
		return;
	}

	if (!EyeCascade.load(EyePath))
	{
		std::cerr << "Error: Could not load cascade classifier XML file." << std::endl;
		return;
	}

	if (!SmileCascade.load(SmilePath))
	{
		std::cerr << "Error: Could not load cascade classifier XML file." << std::endl;
		return;
	}

	DeactivateLayer();
}

PostProcessingFaceDetectionLayer::~PostProcessingFaceDetectionLayer()
{
	std::cout << boost::typeindex::type_id<PostProcessingFaceDetectionLayer>().pretty_name() << " layer instance is destroyed" << std::endl;
}

void PostProcessingFaceDetectionLayer::PostProcessFrame(cv::Mat& InFrame)
{
	Faces.clear();
	Smiles.clear();
	Eyes.clear();

	/*
		Flip the frame
	*/
	cv::flip(InFrame, InFrame, 1);

	FaceCascade.detectMultiScale(InFrame, Faces, 1.1, 4, 0, cv::Size(20, 20));

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

	for (const auto& Face : Faces)
	{
		cv::rectangle(InFrame, Face, cv::Scalar(0, 255, 0), 3);

		SmileCascade.detectMultiScale(InFrame(Face), Smiles, 1.8, 22);

		for (const auto& Smile : Smiles)
		{
			cv::rectangle(InFrame(Face), Smile, cv::Scalar(0, 0, 255), 3);
		}

		EyeCascade.detectMultiScale(InFrame(Face), Eyes, 1.1, 22);

		for (const auto& Eye : Eyes)
		{
			cv::rectangle(InFrame(Face), Eye, cv::Scalar(255, 0, 0), 3);


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
