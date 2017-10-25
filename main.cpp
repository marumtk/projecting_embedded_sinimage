
#include "HighSpeedProjector.h"
#include <opencv2/opencv.hpp>
#include <conio.h>
#include <vector>
#include <string>
#include <math.h>
#include "HSC/CameraUI.hpp"
#include "baslerClass.hpp"
#include <Windows.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <direct.h>
#ifdef _DEBUG
#pragma comment( lib, "opencv_world310d.lib" )
#else
#pragma comment( lib, "opencv_world310.lib" )
#endif

#define proj_width 1024
#define proj_height 768
#define CAMERA_HEIGHT 480
#define CAMERA_WIDTH 640
#define pi 3.14159265358979

//グレイコードを生成する関数
void paint_mask(cv::Mat &paint_image,int num,int pixel_stripe,int mode);
//バイナリコードをグレイコードに変換する関数
int Binary2Gray(int num);
//正弦波パターンを生成する関数
void create_sinimage(std::vector<cv::Mat> &input,int num);

int main()
{
	float embedded_time, frame_rate;
	std::cout << "パターンの埋め込み時間を決定してください" << std::endl;
	std::cin >> embedded_time;
	std::cout << "プロジェクタのフレームレートを決定してください" << std::endl;
	std::cin >> frame_rate;
	if (embedded_time * frame_rate > 1000000) {
		std::cout << "入力が不適切です" << std::endl;
		return -1;
	}
	std::cout << "パラメタ設定完了" << std::endl;



    HighSpeedProjector hsproj;

    // 投影モードの設定
    hsproj.set_projection_mode( PM::MIRROR );               // ミラーモードON
    hsproj.set_projection_mode( PM::ILLUMINANCE_HIGH );     // 高照度モードに変更( 長く投影している場合はONにしちゃだめ )
	
    /**************************************************************************************************************************
    *  通常モード使用例
    ***************************************************************************************************************************/
    /*
    // 投影モードの設定

    // 投影パラメタの設定
    hsproj.set_parameter_value(PP::FRAME_RATE, 1000);
    hsproj.set_parameter_value(PP::BUFFER_MAX_FRAME, 20);
    // 投影映像の生成
    auto image = cv::imread("0000.jpg", 0);
    cv::imshow("image", image);
    cv::waitKey(10);
    hsproj.connect_projector();
    for (;; )
    {
        if (_kbhit() == true) break;
        hsproj.send_image_8bit(image.data);
    }
    */

    /**************************************************************************************************************************
    * バイナリモード使用例
    ***************************************************************************************************************************/
    /*
    // 投影モードの設定
    hsproj.set_projection_mode( PM::BINARY ); //BINARYモードON
    // 投影パラメタの設定
    hsproj.set_parameter_value(PP::FRAME_RATE, 22000u);
    char binary_image[1024 * 768 / 8]; //投影映像生成.
    // 8pixelごとに白い線が入る映像の生成
    // char型の1byteのうちの各bitごとに使用されるので一枚の映像は1024(width) * 768(height) / 8bit(1byte) 分のデータとなる
    for( int i = 0; i < 1024*768/8; i++ )
    {
        binary_image[i] = static_cast< char >( 1 );
    }
    hsproj.connect_projector();
    for( ;; )
    {
        if( _kbhit() == true ) break;
        hsproj.send_image_binary( binary_image );
    }
    */


    /**************************************************************************************************************************
    * パターン埋め込みモード使用例
	* 投影パターンの設定
    ***************************************************************************************************************************/
    
    // 投影モードの設定
    hsproj.set_projection_mode( PM::PATTERN_EMBED ); // パターン埋め込みモードON
    // 投影パラメタの設定
	//下位bitから順に対応
    std::vector< int > led_adjust = { 0, 0, 0, 0, 0, 0, 0, 0 }; //bitごとにLEDの点灯時間を制御
    std::vector< ULONG > bit_sequence = { 0, 1, 2, 4, 7, 5, 3, 6 }; //bitごとの投影順を変更
    hsproj.set_parameter_value(PP::PATTERN_OFFSET, embedded_time); // LSBに〇〇μsのパターンを埋め込み想定
    hsproj.set_parameter_value(PP::FRAME_RATE, frame_rate); //フレームレート
    hsproj.set_parameter_value(PP::BUFFER_MAX_FRAME, 0); //バッファのフレーム数.位相シフトをするときは0に設定
    hsproj.set_parameter_value(PP::BIT_SEQUENCE, bit_sequence );
    hsproj.set_parameter_value(PP::PROJECTOR_LED_ADJUST, led_adjust );

    // 投影映像の生成( LSBが100μsで投影されるからノイズみたいのが入ったような画像が投影される )
	int num = 8; //正弦波の周期数
	std::vector<cv::Mat> input;
	create_sinimage(input, num); //3枚の正弦波パターンの生成(+1枚のreference画像)

	// マスクの生成（チェッカーパターン・グレイコードなどで試すと良い）
	// さらに、マスクを元にして下位ビットにパターンを埋め込む
	cv::Mat pattern_image( cv::Size( proj_width, proj_height ), CV_8UC1, cv::Scalar::all( 255 ) );
	for(int i = 0; i < input.size(); i++ )
	{
		paint_mask(pattern_image, num, (int)proj_width/num, i+1); //とりあえず3bitグレイコード画像をマスクとして生成
		for( int y = 0; y < proj_height; y++ )
		{
			for( int x = 0; x < proj_width; x++ )
			{
				//マスクにおけるピクセルの輝度が0でないとき
				if( pattern_image.data[ y*proj_width+x ] != 0 )
				{
					//最下位ビットだけを1に変更（→明るくなる）
					input[i].data[ y*proj_width+x ] |= 1u;
				}
				//マスクにおけるピクセルの輝度が0であるとき
				else
				{
					//最下位ビットだけを0に変更（→暗くなる）
					input[i].data[ y*proj_width+x ] &= ~1u; 
				}
			}
		}
		//cv::imshow("image"+ std::to_string(i), input[i]);
		//cv::waitKey(10);
	}
	
	


	/**************************************************************************************************************************
	* カメラ周りの設定
	***************************************************************************************************************************/
	// 2台のカメラパラメータを設定
	// 1台目:正弦波パターンを撮像
	cv::Mat image1(cv::Size(CAMERA_WIDTH, CAMERA_HEIGHT), CV_8UC1, cv::Scalar(0, 0, 0));
	basler basler1;
	basler1.connect(0);
	// setParam関連は必ずスタート前に終わらせる 
	basler1.setParam(paramTypeBasler::CaptureType::MonocroGrab);
	basler1.setParam(paramType::HEIGHT, CAMERA_HEIGHT);
	basler1.setParam(paramTypeBasler::Param::ExposureTime, float(1000000/frame_rate)-embedded_time-50.0f);
	basler1.setParam(paramTypeBasler::Param::TriggerDelay, embedded_time); //遅延の設定が必要
	basler1.setParam(paramTypeBasler::AcquisitionMode::TriggerMode);
	// basler1.setParam( paramType::FPS, 500);
	basler1.setParam(paramTypeBasler::FastMode::SensorReadoutModeFast);
	basler1.setParam(paramTypeBasler::GrabStrategy::OneByOne);
	basler1.parameter_all_print();

	// 2台目:グレイコードを撮像
	cv::Mat image2(cv::Size(CAMERA_WIDTH, CAMERA_HEIGHT), CV_8UC1, cv::Scalar(0, 0, 0));
	basler basler2;
	basler2.connect(1);
	basler2.setParam(paramTypeBasler::CaptureType::MonocroGrab);
	basler2.setParam(paramType::WIDTH, CAMERA_WIDTH);
	basler2.setParam(paramTypeBasler::AcquisitionMode::TriggerMode);
	// basler2.setParam( paramType::FPS, 500);
	basler2.setParam(paramTypeBasler::FastMode::SensorReadoutModeFast);
	basler2.setParam(paramTypeBasler::Param::ExposureTime, embedded_time);
	basler2.setParam(paramTypeBasler::GrabStrategy::OneByOne);
	basler2.parameter_all_print();



	/**************************************************************************************************************************
	* 投影・撮像開始
	***************************************************************************************************************************/
	//使用する変数を撮像前にまとめて定義
	std::vector<cv::Mat> tmp1;// カメラ1の撮像画像を格納
	std::vector<cv::Mat> tmp2;// カメラ2の撮像画像を格納
	bool flag = true;

	//2カメラでの撮像開始
	std::thread thr1(
		[&]()
	{
		basler1.start();
		while (flag)
		{
			basler1.captureFrame(image1.data);
			tmp1.push_back(image1.clone());
			//if (frame != 0) tmp1.at(frame1) = image1.clone();
		}
	}
	);

	std::thread thr2(
		[&]()
	{
		basler2.start();
		while (flag)
		{
			basler2.captureFrame(image2.data);
			tmp2.push_back(image2.clone());
			//if (frame != 0) tmp2.at(frame2) = image2.clone();
		}
	}
	);


	//スレッドの立ち上がり待機
	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	//プロジェクタと接続
	hsproj.connect_projector();
	// プロジェクタでの投影開始
	for (;; ) {
		for (int i = 0; i < input.size(); i++) {
			hsproj.send_image_8bit(input[i].data);
		}
		if (_kbhit() == true)  break;
	}


	flag = false;
	thr1.join();
	thr2.join();



	basler1.stop();
	basler1.disconnect();
	basler2.stop();
	basler2.disconnect();


	/**************************************************************************************************************************
	* 撮像画像の保存
	***************************************************************************************************************************/
	//データを保存するフォルダ名を決定
	char object[30];// = "hato_move_yoko2/";
	std::cout << "フォルダ名を入力して下さい" << std::endl;
	std::cin >> object;
	std::cout << "フォルダ名の入力完了" << std::endl;
	std::cout << std::endl;

	//フォルダの作成先の情報
	char preserve_folder[] = "C:/Users/k2vision/Desktop/maruyama/3d_measurement_gray/phase_image/";
	char Folder_name_all[100];
	char Folder_name_cam1[110];
	char Folder_name_cam2[110];
	char Folder_name_3d[110];
	char Filename[120];
	char fileext[] = "bmp";

	//フォルダ作成
	sprintf_s(Folder_name_all, "%s%s", preserve_folder, object);
	if (_mkdir(Folder_name_all) == 0) {
		std::cout << "全データを保存するためのフォルダの作成に成功しました" << std::endl;
	}
	sprintf_s(Folder_name_cam1, "%s%s", Folder_name_all, "/cam1");
	if (_mkdir(Folder_name_cam1) == 0) {
		std::cout << "正弦波画像を保存するためのフォルダの作成に成功しました" << std::endl;
	}
	sprintf_s(Folder_name_cam2, "%s%s", Folder_name_all, "/cam2");
	if (_mkdir(Folder_name_cam2) == 0) {
		std::cout << "グレイコード画像を保存するためのフォルダの作成に成功しました" << std::endl;
	}
	sprintf_s(Folder_name_3d, "%s%s", Folder_name_all, "/3d");
	if (_mkdir(Folder_name_3d) == 0) {
		std::cout << "3dデータを保存するためのフォルダの作成に成功しました" << std::endl;
	}

	std::cout << "画像書き出し開始" << std::endl;
	int output_size = 1000;
	int start_size = 500;
	cv::Mat tmp;
	for (int i = 0; i < output_size; i++){
		sprintf_s(Filename, "%s/%03d.%s", Folder_name_cam1, i, fileext);
		cv::imwrite(Filename, tmp1[i + start_size]);
		sprintf_s(Filename, "%s/%03d.%s", Folder_name_cam2, i, fileext);
		cv::flip(tmp2[i + start_size], tmp, 1);
		cv::imwrite(Filename, tmp);
	}

    return 0;
}



void paint_mask(cv::Mat &paint_image,int num,int pixel_stripe,int mode){
//マスクを元にパターンを埋め込む(グレイコードを生成する関数)
//dataが0のピクセル:暗くなる,dataが255のピクセル:明るくなる
	switch(mode)
	{
	int temp;
		case 1:
			for( int y = 0; y < proj_height; y++ ){
				for( int i = 0; i < num; i++ ){
					temp = Binary2Gray(i);
					for( int x = 0; x < pixel_stripe; x++ ){
						if((temp>>2)%2!=0){
							paint_image.data[y*proj_width + (i*pixel_stripe+x)] = 0;
						}
						else{
							paint_image.data[y*proj_width + (i*pixel_stripe+x)] = 255;
						}
					}
				}
			}
			break;

		case 2:
			for( int y = 0; y < proj_height; y++ ){
				for( int i = 0; i < num; i++ ){
					temp = Binary2Gray(i);
					for( int x = 0; x < pixel_stripe; x++ ){
						if((temp>>1)%2!=0){
							paint_image.data[y*proj_width + (i*pixel_stripe+x)] = 0;
						}
						else{
							paint_image.data[y*proj_width + (i*pixel_stripe+x)] = 255;
						}
					}
				}
			}
			break;

		case 3:
			for( int y = 0; y < proj_height; y++ ){
				for( int i = 0; i < num; i++ ){
					temp = Binary2Gray(i);
					for( int x = 0; x < pixel_stripe; x++ ){
						if(temp%2!=0){
							paint_image.data[y*proj_width + (i*pixel_stripe+x)] = 0;
						}
						else{
							paint_image.data[y*proj_width + (i*pixel_stripe+x)] = 255;
						}
					}
				}
			}
			break;
		//case4では真っ白のパターンを埋め込む
		case 4:
			for (int y = 0; y < proj_height; y++) {
					for (int x = 0; x < proj_width; x++) {
							paint_image.data[y*proj_width + x] = 255;
				}
			}
			break;

		default:
			break;
	}
}

// バイナリコードからグレイコードへの変換
int Binary2Gray(int num){
	int tmp = 0;
	tmp |= num;
	return num ^ (num >> 1);
};

//正弦波パターンの生成
void create_sinimage(std::vector<cv::Mat> &input, int num){
	int wave_length = proj_width/num; //正弦波の波長
	for(int i = 0; i < 4; i++ ){
		cv::Mat img( cv::Size( proj_width, proj_height ), CV_8UC1, cv::Scalar::all( 255 ) );
		if(i!=3){
			for( int y = 0; y < proj_height; y++ ){
				for( int x = 0; x < proj_width; x++ ){
					img.data[ y*proj_width+x ] = int(127.5 + 127.5 * sin(2*pi*x/wave_length + (i-1)*2*pi/3));
				}
			}
		}
		input.push_back(img);
	}
}