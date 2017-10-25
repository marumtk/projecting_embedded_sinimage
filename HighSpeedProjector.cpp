#include "HighSpeedProjector.h"


HighSpeedProjector::HighSpeedProjector()
    :width( 1024 ),
     height( 768 ),
     pDynaFlash( nullptr ),
     nFrameCnt( 0 ),
     pbuffer( nullptr ),
     nProjectionMode( 0 ),
     frame_rate( 1000 ),
     valid_bits( 8 ),
     pattern_offset( 0 ),
     projection_buffer_max_frame( 0 )
{
    /* DynaFlash�N���X�̃C���X�^���X�����Ƃ��ꂼ��̕ϐ��̏����� */
    for( int i = 0; i < 20; i++ ) projector_led_adjust[ i ] = 0;
    for( int i = 0; i < 8; i++ ) bit_sequence[ i ] = i;
    pDynaFlash = CreateDynaFlash();
    if (pDynaFlash == nullptr) throw ( std::runtime_error("Error: DynaFlash�N���X�������s"));
}

HighSpeedProjector::~HighSpeedProjector()
{
    destruct_process();
}

void HighSpeedProjector::destruct_process()
{
    /* ���e�I�� */
    pDynaFlash->Stop();

    /* �t���[���o�b�t�@�̔j�� */
    pDynaFlash->ReleaseFrameBuffer();

    /* �~���[�f�o�C�X���t���[�g��Ԃɂ��� */
    pDynaFlash->Float(0);

    /* DynaFlash�ؒf */
    pDynaFlash->Disconnect();

    /* DynaFlash�N���X�̃C���X�^���X�j�� */
    ReleaseDynaFlash(&pDynaFlash);
}

bool HighSpeedProjector::connect_projector()
{
#ifndef EXTENSION_MODE
    try
    {
        if( !(nProjectionMode & static_cast< ULONG >(PM::BINARY)) == 0 && (nProjectionMode & static_cast< ULONG >(PM::EXT_TRIGGER)) == 0 && (nProjectionMode & static_cast< ULONG >(PM::TRIGGER_SKIP)) == 0 ) throw (static_cast< int >(__LINE__));
    }
    catch( int line )
    {
        std::cout << "Error: line nmber = " << line << std::endl;
        std::cout << "�g���@�\���g�p�������Ƃ���'EXTENSION_MODE'��HighSpeedProjector.h�̈�ԏ��#define���Ă�������" << std::endl;
        std::cout << "���������A���i�łł͎g�p�ł��܂���" << std::endl;
        assert(0);
    }
#endif
    assert( ((nProjectionMode & static_cast< ULONG >(PM::BINARY)) != 0 && (nProjectionMode & static_cast< ULONG >(PM::PATTERN_EMBED)) != 0) == 0 );
    try
    {
        print_all_mode();
        /* ���[�L���O�Z�b�g�ݒ� */
        if (!SetProcessWorkingSetSize(::GetCurrentProcess(), (1000UL * 1024 * 1024), (2000UL * 1024 * 1024))) throw (static_cast< int >(__LINE__));

        /* DynaFlash�ڑ� */
        if (pDynaFlash->Connect(BOARD_INDEX) != STATUS_SUCCESSFUL) throw (static_cast< int >(__LINE__ ));
        /* DynaFlash���Z�b�g */
        if (pDynaFlash->Reset() != STATUS_SUCCESSFUL) throw (static_cast< int >(__LINE__));

        /* �e��o�[�W�����\�� */
        print_version(pDynaFlash);

        /* ���e�p�����[�^�[�ݒ� */
        if( ( nProjectionMode & static_cast< ULONG >( PM::PATTERN_EMBED ) ) == 0 ) { if (pDynaFlash->SetParam(frame_rate, valid_bits, nProjectionMode) != STATUS_SUCCESSFUL) throw (static_cast< int >(__LINE__)); }
        else { if (pDynaFlash->SetParam( frame_rate, valid_bits, bit_sequence, pattern_offset, projector_led_adjust, nProjectionMode ) != STATUS_SUCCESSFUL) throw (static_cast< int >(__LINE__)); }

        /* �Ɠx�ݒ� */
        if( ( nProjectionMode & static_cast< ULONG >( PM::ILLUMINANCE_HIGH ) ) == 0 ) { if (pDynaFlash->SetIlluminance(LOW_MODE) != STATUS_SUCCESSFUL) throw (static_cast< int >(__LINE__)); }
        else { if ( pDynaFlash->SetIlluminance(HIGH_MODE) != STATUS_SUCCESSFUL) throw(static_cast< int >(__LINE__)); }

        /* ���e�p�t���[���o�b�t�@�擾 */
        if (pDynaFlash->AllocFrameBuffer(ALLOC_FRAME_BUFFER) != STATUS_SUCCESSFUL) throw (static_cast< int >(__LINE__));

        /* ���e�f�[�^�X�V�\�ȓ��e�p�t���[���o�b�t�@�擾 */
        pDynaFlash->GetFrameBuffer(&pbuffer, &nFrameCnt);

        /* ���e�f�[�^������ */
        for (int i = 0; i < ALLOC_FRAME_BUFFER; i++) 
        {
            if( (nProjectionMode & static_cast< ULONG >(PM::BINARY)) == 0 )
            {
                memcpy(pbuffer, ini_buf, FRAME_BUF_SIZE_8BIT);
            }
            else
            {
                memcpy(pbuffer, ini_buf_bin, FRAME_BUF_SIZE_BINARY);
            }
        }

        /* ���e�J�n */
        if (pDynaFlash->Start() != STATUS_SUCCESSFUL) throw (static_cast< int >(__LINE__));

        return true;
    }
    catch( int line )
    {
        std::cout << "Error: line_number = " << line << std::endl;
        destruct_process();
        return false;
    }
}


void HighSpeedProjector::send_image_8bit( const void *data )
{
    auto send_image = [ & ]()
    {
        for (;;)
        {
            /* ���e�f�[�^�X�V�\�ȓ��e�p�t���[���o�b�t�@�擾 */
            if (pDynaFlash->GetFrameBuffer(&pbuffer, &nFrameCnt) != STATUS_SUCCESSFUL) {
                throw (static_cast< int >(__LINE__));
            }
            /* �K�v�ł���΂����Ƀt���[���o�b�t�@�̍X�V�������L�ڂ��� */
            if ((pbuffer != nullptr) && (nFrameCnt != 0))
            {
                memcpy(pbuffer, data, FRAME_BUF_SIZE_8BIT);
                /* DynaFlash�֓]������ */
                if (pDynaFlash->PostFrameBuffer(1) != STATUS_SUCCESSFUL) throw (static_cast< int >(__LINE__));
                break;
            }
        }
    };
    try
    {
        if( projection_buffer_max_frame == 0 )
        {
            send_image();
        }
        else
        {
            ULONG input_frame_tmp = 0;
            ULONG output_frame_tmp = 0;
            get_status( input_frame_tmp, output_frame_tmp );
            if( static_cast< unsigned int >( input_frame_tmp - output_frame_tmp ) < projection_buffer_max_frame ) send_image();
        }
    }
    catch( int line )
    {
        std::cout << "Error: line_number = " << line << std::endl;
        destruct_process();
    }
}

void HighSpeedProjector::send_image_binary(const void *data)
{
    try
    {
        for (;;)
        {
            /* ���e�f�[�^�X�V�\�ȓ��e�p�t���[���o�b�t�@�擾 */
            if (pDynaFlash->GetFrameBuffer(&pbuffer, &nFrameCnt) != STATUS_SUCCESSFUL) {
                throw (static_cast< int >(__LINE__));
            }
            /* �K�v�ł���΂����Ƀt���[���o�b�t�@�̍X�V�������L�ڂ��� */
            if ((pbuffer != nullptr) && (nFrameCnt != 0))
            {
                memcpy(pbuffer, data, FRAME_BUF_SIZE_BINARY);
                /* DynaFlash�֓]������ */
                if (pDynaFlash->PostFrameBuffer(1) != STATUS_SUCCESSFUL) throw (static_cast< int >(__LINE__));
                break;
            }
        }
    }
    catch (int line)
    {
        std::cout << "Error: line_number = " << line << std::endl;
        destruct_process();
    }
}

void HighSpeedProjector::print_version(CDynaFlash *pDynaFlash)
{
    char DriverVersion[40];
    unsigned long nVersion;

    /* �h���C�o�o�[�W�����擾 */
    pDynaFlash->GetDriverVersion(DriverVersion);
    printf("DRIVER Ver : %s\r\n", DriverVersion);

    /* HW�o�[�W�����擾 */
    pDynaFlash->GetHWVersion(&nVersion);
    printf("HW Ver     : %08x\r\n", nVersion);

    /* DLL�o�[�W�����擾 */
    pDynaFlash->GetDLLVersion(&nVersion);
    printf("DLL Ver    : %08x\r\n", nVersion);
}

void HighSpeedProjector::get_status( ULONG &out_input_frame_count, ULONG &out_output_frame_count )
{
    DYNAFLASH_STATUS pDynaFlashStatus;
    pDynaFlash->GetStatus( &pDynaFlashStatus );
    out_input_frame_count = pDynaFlashStatus.InputFrames;
    out_output_frame_count = pDynaFlashStatus.OutputFrames;
}

void HighSpeedProjector::set_projection_mode(PROJECTION_MODE projection_mode )
{
    nProjectionMode |= static_cast< ULONG >( projection_mode );
}


void HighSpeedProjector::set_parameter_value( const PROJECTION_PARAMETER in_set_parameter_index, const ULONG in_set_parameter )
{
    switch( in_set_parameter_index )
    {
    case PP::FRAME_RATE : 
        {  
            frame_rate = in_set_parameter; 
            break;
        }
    case PP::BUFFER_MAX_FRAME :
        {
            projection_buffer_max_frame = in_set_parameter;
            break;
        }
    case PP::PATTERN_OFFSET : 
        {
            assert((nProjectionMode & static_cast< ULONG >(PM::PATTERN_EMBED)) != 0);
            pattern_offset = in_set_parameter;
            break;
        }
    }
}

void HighSpeedProjector::set_parameter_value(const PROJECTION_PARAMETER in_set_parameter_index, const std::vector< ULONG > in_set_parameter)
{
    assert((nProjectionMode & static_cast< ULONG >(PM::PATTERN_EMBED)) != 0);
    assert( in_set_parameter.size() == 8u );
    for( int i = 0; i < in_set_parameter.size(); i++ ) bit_sequence[ i ] = in_set_parameter[ i ];
}

void HighSpeedProjector::set_parameter_value(const PROJECTION_PARAMETER in_set_parameter_index, const std::vector< INT > in_set_parameter)
{
    assert((nProjectionMode & static_cast< ULONG >(PM::PATTERN_EMBED)) != 0);
    for (int i = 0; i < in_set_parameter.size(); i++) projector_led_adjust[i] = in_set_parameter[i];
}

void HighSpeedProjector::print_all_mode()
{
    if ((nProjectionMode & static_cast< ULONG >(PM::MIRROR)) != 0) std::cout << "MIRROR MODE : ON" << std::endl;
    else std::cout << "MIRROR MODE : OFF" << std::endl;
    if ((nProjectionMode & static_cast< ULONG >(PM::FLIP)) != 0) std::cout << "FLIP MODE : ON" << std::endl;
    else std::cout << "FLIP MODE : OFF" << std::endl;
    if ((nProjectionMode & static_cast< ULONG >(PM::COMP)) != 0) std::cout << "COMP MODE : ON" << std::endl;
    else std::cout << "COMP MODE : OFF" << std::endl;
    if ((nProjectionMode & static_cast< ULONG >(PM::ONESHOT)) != 0) std::cout << "ONESHOT MODE : ON" << std::endl;
    else std::cout << "ONESHOT MODE : OFF" << std::endl;
    if ((nProjectionMode & static_cast< ULONG >(PM::BINARY)) != 0) std::cout << "BINARY MODE : ON" << std::endl;
    else std::cout << "BINARY MODE : OFF" << std::endl;
    if ((nProjectionMode & static_cast< ULONG >(PM::EXT_TRIGGER)) != 0) std::cout << "EXT_TRIGGER MODE : ON" << std::endl;
    else std::cout << "EXT_TRIGGER MODE : OFF" << std::endl;
    if ((nProjectionMode & static_cast< ULONG >(PM::TRIGGER_SKIP)) != 0) std::cout << "TRIGGER_SKIP : ON" << std::endl;
    else std::cout << "TRIGGER_SKIP : OFF" << std::endl;
    if((nProjectionMode & static_cast< ULONG >(PM::PATTERN_EMBED)) != 0) std::cout << "PATTERN_EMBED MODE : ON" << std::endl;
    else std::cout << "PATTERN_EMBED MODE: OFF" << std::endl;
    if ((nProjectionMode & static_cast< ULONG >(PM::ILLUMINANCE_HIGH)) != 0) std::cout << "ILLUMINANCE_HIGH MODE : ON" << std::endl;
    else std::cout << "ILLUMINANCE_HIGH MODE : OFF" << std::endl;
    std::cout << "FRAME_RATE = " << frame_rate << " [fps] " << std::endl;
    if( projection_buffer_max_frame == 0 ) std::cout << "BUFFER_MAX_FRAME : No set" << std::endl;
    else std::cout << "BUFFER_MAX_FRAME = " << projection_buffer_max_frame << " [frame] " << std::endl;
    if( pattern_offset == 0  ) std::cout << "PATTERN_OFFSET : No set" << std::endl;
    else std::cout << "PATTERN_OFFSET = " << pattern_offset << " [us] " << std::endl;
    std::cout << "BIT_SEQUENCE = [ " ;
    for( auto tmp : bit_sequence ) std::cout << tmp << ", ";
    std::cout << " ] " << std::endl;
    std::cout << "PROJECTOR_LED_ADJUST = [ ";
    for ( int i = 0; i < 8; i++ ) std::cout << projector_led_adjust[ i ] << ", ";
    std::cout << " ] " << std::endl;
}
