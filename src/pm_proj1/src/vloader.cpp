#include "pm_proj1.h"

geometry_msgs::Vector3 center_tracked;

void cbPose_track(const geometry_msgs::Vector3 &msg)
{
    // Received center
    // FAZER direiro o subscriber
    center_tracked.x = msg.x;
    center_tracked.y = msg.y;
    ROS_WARN_STREAM("Inside callback tracker!");
    ROS_WARN_STREAM("center=(" << msg.x << "," << msg.y << ")");
}

/////////////////////////EXERCISE 1/////////////////////////////////////

int main(int argc, char **argv)
{

    ros::init(argc, argv, "vloader");
    // Create the node handles to establish the program as a ROS node

    ros::NodeHandle n_private("~");

    n_private.param<int>("lowH", lowH, 0);
    n_private.param<int>("lowS", lowS, 0);
    n_private.param<int>("lowV", lowV, 0);
    n_private.param<int>("highH", highH, 180);
    n_private.param<int>("highS", highS, 255);
    n_private.param<int>("highV", highV, 255);
    n_private.param<std::string>("path", path, "../project_pm/src/pm_proj1/src/videoPlastic.mp4");

    // Publishers
    ros::NodeHandle n_frame;
    ros::NodeHandle n_center;
    ros::NodeHandle n_center_tracked;

    ros::Subscriber sub = n_center_tracked.subscribe("vtracker/ball_center_tracker", 1, cbPose_track);

    image_transport::ImageTransport it(n_frame);

    ros::Publisher center_ball_publisher;

    image_transport::Publisher frame_publisher;

    frame_publisher = it.advertise("vloarder/frame", 1);
    center_ball_publisher = n_center.advertise<geometry_msgs::Vector3>("vloarder/ball_center", 1000);
    ros::Rate loop_rate(10);

    // save videos

    if (path == "../project_pm/src/pm_proj1/src/videoPlastic.mp4")
    {
        name = "p1_plastic.mp4";
        name2 = "p2_plastic.mp4";
        name3 = "p3_plastic.mp4";
    }
    else if (path == "../project_pm/src/pm_proj1/src/videoTennis.mp4")
    {
        name = "p1_tennis.mp4";
        name2 = "p2_tennis.mp4";
        name3 = "p3_tennis.mp4";
    }
    int count = 0;

    // Open video
    cv::VideoCapture cap(path);

    // Check if video opened successfully
    if (!cap.isOpened())
    {
        std::cout << "Error opening video stream or file" << std::endl;
        return -1;
    }

    // naming windows
    cv::namedWindow(window_capture_name, cv::WINDOW_NORMAL);
    cv::namedWindow(window_detection_name, cv::WINDOW_NORMAL);
    cv::namedWindow(window_cont_name, cv::WINDOW_NORMAL);

    // declarations open cv
    cv::Mat frame, frame_HSV, frame_threshold, cont;
    int fps = 30;

    // Acquire input size
    cv::Size frame_size = cv::Size((int)cap.get(cv::CAP_PROP_FRAME_WIDTH), (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT));

    cv::VideoWriter writer(name, cv::VideoWriter::fourcc('a', 'v', 'c', '1'), fps, frame_size);

    cv::VideoWriter writer2(name2, cv::VideoWriter::fourcc('a', 'v', 'c', '1'), fps, frame_size);

    cv::VideoWriter writer3(name3, cv::VideoWriter::fourcc('a', 'v', 'c', '1'), fps, frame_size);

    cv::Scalar color = cv::Scalar(0, 255, 0);
    cv::Scalar color2 = cv::Scalar(255, 0, 0);
    cv::Scalar color3 = cv::Scalar(0, 0, 255);
    while (ros::ok)
    {
        // Iterate through frames
        while (cap.isOpened())
        {

            ///////////////////Alinea A//////////////////////
            // Capture next frame
            cap >> frame;
            double area = 0;
            // If the frame is empty, break immediately
            if (frame.empty())
            {
                break;
            }
            cv::Mat prin;
            sensor_msgs::ImagePtr msg_frame = cv_bridge::CvImage(std_msgs::Header(), "bgr8", frame).toImageMsg();
            frame_publisher.publish(msg_frame);
            ////////////////////Alinea B//////////////////////
            // Convert from BGR to HSV colorspace
            cvtColor(frame, frame_HSV, cv::COLOR_BGR2HSV);
            // Detect the object based on HSV Range Values

            inRange(frame_HSV, cv::Scalar(lowH, lowS, lowV), cv::Scalar(highH, highS, highV), frame_threshold);
            cv::dilate(frame_threshold, frame_threshold, cv::Mat(), cv::Point(-1, 1), 4);
            
            prin = frame_threshold.clone();
            geometry_msgs::Vector3 center;
            cv::bitwise_and(frame, frame, prin, frame_threshold);
            ///////////////////// Alinea D //////////////////////////////
            cv::Moments m = moments(frame_threshold, false);
            cv::Point com(m.m10 / m.m00, m.m01 / m.m00);
            cv::drawMarker(frame, com, color, cv::MARKER_CROSS, 20, 5);
            cv::drawMarker(frame_threshold, com, color, cv::MARKER_CROSS, 20, 5);
            cv::putText(frame, "Obs", cv::Point(com.x - 0, com.y - 50), cv::FONT_HERSHEY_DUPLEX, 1, color);

            cv::Point tracked;
            tracked.x = center_tracked.x;
            tracked.y = center_tracked.y;
            cv::drawMarker(frame, tracked, color2, cv::MARKER_TRIANGLE_DOWN, 20, 5);
            cv::putText(frame, "Tracked", cv::Point(tracked.x - 100, tracked.y - 100), cv::FONT_HERSHEY_DUPLEX, 1, color2);

            cv::Mat copy;
            copy = frame.clone();

            if (tracked.y >= 550 && path == "../project_pm/src/pm_proj1/src/videoPlastic.mp4")
            {
                cv::putText(copy, "COLLISION", cv::Point(900, 30), cv::FONT_HERSHEY_DUPLEX, 1, color3);
            }

            center.x = com.x;
            center.y = com.y;
            center.z = 0;

            center_ball_publisher.publish(center);

            Canny(frame_threshold, cont, thresh, thresh * 2);
            cv::cvtColor(cont, prin, cv::COLOR_GRAY2BGR);
            std::vector<std::vector<cv::Point>> contours;
            std::vector<cv::Vec4i> hierarchy;
            findContours(cont, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
            cv::Mat drawing = cv::Mat::zeros(cont.size(), CV_8UC3);
            for (size_t i = 0; i < contours.size(); i++)
            {
                cv::Scalar color = cv::Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
                drawContours(frame_threshold, contours, (int)i, color, 2, cv::LINE_8, hierarchy, 0);
                cv::Rect r;
                r = cv::boundingRect(contours[i]);
                //////////////// Alinea E///////////////////
                area = contourArea(contours[i]);
                if (area > 20000)
                {
                    int dif = r.width-r.height;
                    cv::rectangle(copy, r, CV_RGB(0, 255, 0), 2);
                    vec.push_back(area);
                    if ( (dif >42 )|| (dif < 17 && r.width >223 )&& path == "../project_pm/src/pm_proj1/src/videoTennis.mp4")
                    {
                        cv::putText(copy, "COLLISION", cv::Point(450, 30), cv::FONT_HERSHEY_DUPLEX, 1, color3);
                    }
                }
            }

            // Show the frames
            imshow(window_capture_name, frame);
            imshow(window_detection_name, frame_threshold);
            imshow(window_cont_name, prin);
            moveWindow(window_capture_name, 800, 10);
            moveWindow(window_detection_name, 150, 250);
            moveWindow(window_cont_name, 800, 800);
            resizeWindow(window_capture_name, 950, 550);
            resizeWindow(window_detection_name, 600, 600);
            resizeWindow(window_cont_name, 950, 550);

            writer.write(prin);
            writer2.write(frame);
            writer3.write(copy);
            // Press Space Bar to continue, ESC to exit
            char c = (char)cv::waitKey(0);
            if (c == 27)
                break;

            ros::spinOnce();
            loop_rate.sleep();
        }
        char c = (char)cv::waitKey(0);
        if (c == 27)
            break;
    }
    double sum = 0;
    int size = 1;
    double avg_area;
    for (int i = 0; i < vec.size(); i++)
    {
        sum += vec[i];
        size++;
    }
    avg_area = sum / size;
    std::cout << " Average Area: " << avg_area << std::endl;
    // When everything is done, release the video capture and writer objects
    cap.release();
    writer.release();
    writer2.release();
    writer3.release();
    // Close all the frames
    cv::destroyAllWindows();

    return 0;
}
