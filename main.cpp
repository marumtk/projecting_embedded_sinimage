
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

//�O���C�R�[�h�𐶐�����֐�
void paint_mask(cv::Mat &paint_image,int num,int pixel_stripe,int mode);
//�o�C�i���R�[�h���O���C�R�[�h�ɕϊ�����֐�
int Binary2Gray(int num);
//�����g�p�^�[���𐶐�����֐�
void create_sinimage(std::vector<cv::Mat> &input,int num);

int main()
{
	float embedded_time, frame_rate;
	std::cout << "�p�^�[���̖��ߍ��ݎ��Ԃ����肵�Ă�������" << std::endl;
	std::cin >> embedded_time;
	std::cout << "�v���W�F�N�^�̃t���[�����[�g�����肵�Ă�������" << std::endl;
	std::cin >> frame_rate;
	if (embedded_time * frame_rate > 1000000) {
		std::cout << "���͂��s�K�؂ł�" << std::endl;
		return -1;
	}
	std::cout << "�p�����^�ݒ芮��" << std::endl;



    HighSpeedProjector hsproj;

    // ���e���[�h�̐ݒ�
    hsproj.set_projection_mode( PM::MIRROR );               // �~���[���[�hON
    hsproj.set_projection_mode( PM::ILLUMINANCE_HIGH );     // ���Ɠx���[�h�ɕύX( �������e���Ă���ꍇ��ON�ɂ����Ⴞ�� )
	
    /**************************************************************************************************************************
    *  �ʏ탂�[�h�g�p��
    ***************************************************************************************************************************/
    /*
    // ���e���[�h�̐ݒ�

    // ���e�p�����^�̐ݒ�
    hsproj.set_parameter_value(PP::FRAME_RATE, 1000);
    hsproj.set_parameter_value(PP::BUFFER_MAX_FRAME, 20);
    // ���e�f���̐���
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
    * �o�C�i�����[�h�g�p��
    ***************************************************************************************************************************/
    /*
    // ���e���[�h�̐ݒ�
    hsproj.set_projection_mode( PM::BINARY ); //BINARY���[�hON
    // ���e�p�����^�̐ݒ�
    hsproj.set_parameter_value(PP::FRAME_RATE, 22000u);
    char binary_image[1024 * 768 / 8]; //���e�f������.
    // 8pixel���Ƃɔ�����������f���̐���
    // char�^��1byte�̂����̊ebit���ƂɎg�p�����̂ňꖇ�̉f����1024(width) * 768(height) / 8bit(1byte) ���̃f�[�^�ƂȂ�
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
    * �p�^�[�����ߍ��݃��[�h�g�p��
	* ���e�p�^�[���̐ݒ�
    ***************************************************************************************************************************/
    
    // ���e���[�h�̐ݒ�
    hsproj.set_projection_mode( PM::PATTERN_EMBED ); // �p�^�[�����ߍ��݃��[�hON
    // ���e�p�����^�̐ݒ�
	//����bit���珇�ɑΉ�
    std::vector< int > led_adjust = { 0, 0, 0, 0, 0, 0, 0, 0 }; //bit���Ƃ�LED�̓_�����Ԃ𐧌�
    std::vector< ULONG > bit_sequence = { 0, 1, 2, 4, 7, 5, 3, 6 }; //bit���Ƃ̓��e����ύX
    hsproj.set_parameter_value(PP::PATTERN_OFFSET, embedded_time); // LSB�ɁZ�Z��s�̃p�^�[���𖄂ߍ��ݑz��
    hsproj.set_parameter_value(PP::FRAME_RATE, frame_rate); //�t���[�����[�g
    hsproj.set_parameter_value(PP::BUFFER_MAX_FRAME, 0); //�o�b�t�@�̃t���[����.�ʑ��V�t�g������Ƃ���0�ɐݒ�
    hsproj.set_parameter_value(PP::BIT_SEQUENCE, bit_sequence );
    hsproj.set_parameter_value(PP::PROJECTOR_LED_ADJUST, led_adjust );

    // ���e�f���̐���( LSB��100��s�œ��e����邩��m�C�Y�݂����̂��������悤�ȉ摜�����e����� )
	int num = 8; //�����g�̎�����
	std::vector<cv::Mat> input;
	create_sinimage(input, num); //3���̐����g�p�^�[���̐���(+1����reference�摜)

	// �}�X�N�̐����i�`�F�b�J�[�p�^�[���E�O���C�R�[�h�ȂǂŎ����Ɨǂ��j
	// ����ɁA�}�X�N�����ɂ��ĉ��ʃr�b�g�Ƀp�^�[���𖄂ߍ���
	cv::Mat pattern_image( cv::Size( proj_width, proj_height ), CV_8UC1, cv::Scalar::all( 255 ) );
	for(int i = 0; i < input.size(); i++ )
	{
		paint_mask(pattern_image, num, (int)proj_width/num, i+1); //�Ƃ肠����3bit�O���C�R�[�h�摜���}�X�N�Ƃ��Đ���
		for( int y = 0; y < proj_height; y++ )
		{
			for( int x = 0; x < proj_width; x++ )
			{
				//�}�X�N�ɂ�����s�N�Z���̋P�x��0�łȂ��Ƃ�
				if( pattern_image.data[ y*proj_width+x ] != 0 )
				{
					//�ŉ��ʃr�b�g������1�ɕύX�i�����邭�Ȃ�j
					input[i].data[ y*proj_width+x ] |= 1u;
				}
				//�}�X�N�ɂ�����s�N�Z���̋P�x��0�ł���Ƃ�
				else
				{
					//�ŉ��ʃr�b�g������0�ɕύX�i���Â��Ȃ�j
					input[i].data[ y*proj_width+x ] &= ~1u; 
				}
			}
		}
		//cv::imshow("image"+ std::to_string(i), input[i]);
		//cv::waitKey(10);
	}
	
	


	/**************************************************************************************************************************
	* �J��������̐ݒ�
	***************************************************************************************************************************/
	// 2��̃J�����p�����[�^��ݒ�
	// 1���:�����g�p�^�[�����B��
	cv::Mat image1(cv::Size(CAMERA_WIDTH, CAMERA_HEIGHT), CV_8UC1, cv::Scalar(0, 0, 0));
	basler basler1;
	basler1.connect(0);
	// setParam�֘A�͕K���X�^�[�g�O�ɏI��点�� 
	basler1.setParam(paramTypeBasler::CaptureType::MonocroGrab);
	basler1.setParam(paramType::HEIGHT, CAMERA_HEIGHT);
	basler1.setParam(paramTypeBasler::Param::ExposureTime, float(1000000/frame_rate)-embedded_time-50.0f);
	basler1.setParam(paramTypeBasler::Param::TriggerDelay, embedded_time); //�x���̐ݒ肪�K�v
	basler1.setParam(paramTypeBasler::AcquisitionMode::TriggerMode);
	// basler1.setParam( paramType::FPS, 500);
	basler1.setParam(paramTypeBasler::FastMode::SensorReadoutModeFast);
	basler1.setParam(paramTypeBasler::GrabStrategy::OneByOne);
	basler1.parameter_all_print();

	// 2���:�O���C�R�[�h���B��
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
	* ���e�E�B���J�n
	***************************************************************************************************************************/
	//�g�p����ϐ����B���O�ɂ܂Ƃ߂Ē�`
	std::vector<cv::Mat> tmp1;// �J����1�̎B���摜���i�[
	std::vector<cv::Mat> tmp2;// �J����2�̎B���摜���i�[
	bool flag = true;

	//2�J�����ł̎B���J�n
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


	//�X���b�h�̗����オ��ҋ@
	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	//�v���W�F�N�^�Ɛڑ�
	hsproj.connect_projector();
	// �v���W�F�N�^�ł̓��e�J�n
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
	* �B���摜�̕ۑ�
	***************************************************************************************************************************/
	//�f�[�^��ۑ�����t�H���_��������
	char object[30];// = "hato_move_yoko2/";
	std::cout << "�t�H���_������͂��ĉ�����" << std::endl;
	std::cin >> object;
	std::cout << "�t�H���_���̓��͊���" << std::endl;
	std::cout << std::endl;

	//�t�H���_�̍쐬��̏��
	char preserve_folder[] = "C:/Users/k2vision/Desktop/maruyama/3d_measurement_gray/phase_image/";
	char Folder_name_all[100];
	char Folder_name_cam1[110];
	char Folder_name_cam2[110];
	char Folder_name_3d[110];
	char Filename[120];
	char fileext[] = "bmp";

	//�t�H���_�쐬
	sprintf_s(Folder_name_all, "%s%s", preserve_folder, object);
	if (_mkdir(Folder_name_all) == 0) {
		std::cout << "�S�f�[�^��ۑ����邽�߂̃t�H���_�̍쐬�ɐ������܂���" << std::endl;
	}
	sprintf_s(Folder_name_cam1, "%s%s", Folder_name_all, "/cam1");
	if (_mkdir(Folder_name_cam1) == 0) {
		std::cout << "�����g�摜��ۑ����邽�߂̃t�H���_�̍쐬�ɐ������܂���" << std::endl;
	}
	sprintf_s(Folder_name_cam2, "%s%s", Folder_name_all, "/cam2");
	if (_mkdir(Folder_name_cam2) == 0) {
		std::cout << "�O���C�R�[�h�摜��ۑ����邽�߂̃t�H���_�̍쐬�ɐ������܂���" << std::endl;
	}
	sprintf_s(Folder_name_3d, "%s%s", Folder_name_all, "/3d");
	if (_mkdir(Folder_name_3d) == 0) {
		std::cout << "3d�f�[�^��ۑ����邽�߂̃t�H���_�̍쐬�ɐ������܂���" << std::endl;
	}

	std::cout << "�摜�����o���J�n" << std::endl;
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
//�}�X�N�����Ƀp�^�[���𖄂ߍ���(�O���C�R�[�h�𐶐�����֐�)
//data��0�̃s�N�Z��:�Â��Ȃ�,data��255�̃s�N�Z��:���邭�Ȃ�
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
		//case4�ł͐^�����̃p�^�[���𖄂ߍ���
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

// �o�C�i���R�[�h����O���C�R�[�h�ւ̕ϊ�
int Binary2Gray(int num){
	int tmp = 0;
	tmp |= num;
	return num ^ (num >> 1);
};

//�����g�p�^�[���̐���
void create_sinimage(std::vector<cv::Mat> &input, int num){
	int wave_length = proj_width/num; //�����g�̔g��
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