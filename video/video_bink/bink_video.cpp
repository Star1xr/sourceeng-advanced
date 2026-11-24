//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================

#include "bink_video.h"
#include "video_macros.h"

#include "filesystem.h"
#include "tier0/icommandline.h"
#include "tier1/strtools.h"
#include "tier1/utllinkedlist.h"
#include "tier1/KeyValues.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/MaterialSystemUtil.h"
#include "materialsystem/itexture.h"
#include "vtf/vtf.h"
#include "pixelwriter.h"
#include "tier2/tier2.h"
#include "platform.h"


#include "SDL.h"
#include "bink_material.h"
#include "tier0/memdbgon.h"

// ===========================================================================
// Singleton to expose Bink video subsystem
// ===========================================================================
static CBinkVideoSubSystem g_BinkSystem;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CBinkVideoSubSystem, IVideoSubSystem, VIDEO_SUBSYSTEM_INTERFACE_VERSION, g_BinkSystem );


// ===========================================================================
// List of file extensions and features supported by this subsystem
// ===========================================================================
VideoFileExtensionInfo_t s_BinkExtensions[] = 
{
	{ ".bik", VideoSystem::BINK,  VideoSystemFeature::FULL_PLAYBACK },
};

const int s_BinkExtensionCount = ARRAYSIZE( s_BinkExtensions );
const VideoSystemFeature_t	CBinkVideoSubSystem::DEFAULT_FEATURE_SET = VideoSystemFeature::FULL_PLAYBACK;

// ===========================================================================
// CBinkVideoSubSystem class
// ===========================================================================
CBinkVideoSubSystem::CBinkVideoSubSystem() :
	m_bBinkInitialized( false ),
	m_LastResult( VideoResult::SUCCESS ),
	m_CurrentStatus( VideoSystemStatus::NOT_INITIALIZED ),
	m_AvailableFeatures( CBinkVideoSubSystem::DEFAULT_FEATURE_SET ),
	m_pCommonServices( nullptr )
{

}

CBinkVideoSubSystem::~CBinkVideoSubSystem()
{
	ShutdownBink();		// Super redundant safety check
}

// ===========================================================================
// IAppSystem methods
// ===========================================================================
bool CBinkVideoSubSystem::Connect( CreateInterfaceFn factory )
{
	if ( !BaseClass::Connect( factory ) )
	{
		return false;
	}

	if ( g_pFullFileSystem == nullptr || materials == nullptr ) 
	{
		Msg( "Bink video subsystem failed to connect to missing a required system\n" );
		return false;
	}
	return true;
}

void CBinkVideoSubSystem::Disconnect()
{
	BaseClass::Disconnect();
}

void* CBinkVideoSubSystem::QueryInterface( const char *pInterfaceName )
{

	if ( IS_NOT_EMPTY( pInterfaceName ) )
	{
		if ( V_strncmp(	pInterfaceName, VIDEO_SUBSYSTEM_INTERFACE_VERSION, Q_strlen( VIDEO_SUBSYSTEM_INTERFACE_VERSION ) + 1) == STRINGS_MATCH )
		{
			return (IVideoSubSystem*) this;
		}
	}

	return nullptr;
}


InitReturnVal_t CBinkVideoSubSystem::Init()
{
	InitReturnVal_t nRetVal = BaseClass::Init();
	if ( nRetVal != INIT_OK )
	{
		return nRetVal;
	}

	return INIT_OK;

}

void CBinkVideoSubSystem::Shutdown()
{
	// Make sure we shut down Bink
	ShutdownBink();

	BaseClass::Shutdown();
}


// ===========================================================================
// IVideoSubSystem identification methods  
// ===========================================================================
VideoSystem_t CBinkVideoSubSystem::GetSystemID()
{
	return VideoSystem::BINK;
}


VideoSystemStatus_t CBinkVideoSubSystem::GetSystemStatus()
{
	return m_CurrentStatus;
}


VideoSystemFeature_t CBinkVideoSubSystem::GetSupportedFeatures()
{
	return m_AvailableFeatures;
}


const char* CBinkVideoSubSystem::GetVideoSystemName()
{
	return "BINK";
}


// ===========================================================================
// IVideoSubSystem setup and shutdown services
// ===========================================================================
bool CBinkVideoSubSystem::InitializeVideoSystem( IVideoCommonServices *pCommonServices )
{
	m_AvailableFeatures = DEFAULT_FEATURE_SET;			// Put here because of issue with static const int, binary OR and DEBUG builds

	AssertPtr( pCommonServices );
	m_pCommonServices = pCommonServices;

	return ( m_bBinkInitialized ) ? true : SetupBink();
}


bool CBinkVideoSubSystem::ShutdownVideoSystem()
{
	return (  m_bBinkInitialized ) ? ShutdownBink() : true;
}


VideoResult_t CBinkVideoSubSystem::VideoSoundDeviceCMD( VideoSoundDeviceOperation_t operation, void *pDevice, void *pData )
{
	switch ( operation ) 
	{
		case VideoSoundDeviceOperation::SET_DIRECT_SOUND_DEVICE:
		{
			return SetResult( VideoResult::OPERATION_NOT_SUPPORTED );
		}

		case VideoSoundDeviceOperation::SET_MILES_SOUND_DEVICE:
		case VideoSoundDeviceOperation::HOOK_X_AUDIO:
		{
			return SetResult( VideoResult::OPERATION_NOT_SUPPORTED );
		}

		default:
		{
			return SetResult( VideoResult::UNKNOWN_OPERATION );
		}
	}
}


// ===========================================================================
// IVideoSubSystem supported extensions & features
// ===========================================================================
int CBinkVideoSubSystem::GetSupportedFileExtensionCount()
{
	return s_BinkExtensionCount;
}

 
const char* CBinkVideoSubSystem::GetSupportedFileExtension( int num )
{
	return ( num < 0 || num >= s_BinkExtensionCount ) ? nullptr : s_BinkExtensions[num].m_FileExtension;
}

 
VideoSystemFeature_t CBinkVideoSubSystem::GetSupportedFileExtensionFeatures( int num )
{
	 return ( num < 0 || num >= s_BinkExtensionCount ) ? VideoSystemFeature::NO_FEATURES : s_BinkExtensions[num].m_VideoFeatures;
}



// ===========================================================================
// Minimal self-contained audio player for fullscreen movies: decodes the audio
// stream with ffmpeg inside the SDL audio callback of its own SDL device.
// ===========================================================================
struct BinkFSAudio
{
	AVFormatContext *fmt = nullptr;
	AVCodecContext *ctx = nullptr;
	AVPacket *pkt = nullptr;
	AVFrame *frm = nullptr;
	int stream = -1;
	SDL_AudioDeviceID dev = 0;
	int16_t *buf = nullptr;
	int bufCap = 0, bufLen = 0, bufPos = 0;	// in stereo frames
	bool eof = false;

	static float Sample( const AVFrame *f, int c, int i )
	{
		int ch = f->ch_layout.nb_channels;
		switch ( f->format )
		{
		case AV_SAMPLE_FMT_FLTP: return ((float*)f->extended_data[c])[i];
		case AV_SAMPLE_FMT_FLT:  return ((float*)f->data[0])[i*ch+c];
		case AV_SAMPLE_FMT_S16P: return ((int16_t*)f->extended_data[c])[i] / 32768.0f;
		case AV_SAMPLE_FMT_S16:  return ((int16_t*)f->data[0])[i*ch+c] / 32768.0f;
		}
		return 0.0f;
	}

	bool Decode()
	{
		while ( !eof )
		{
			int r = avcodec_receive_frame( ctx, frm );
			if ( r == 0 )
			{
				int n = frm->nb_samples, ch = frm->ch_layout.nb_channels;
				if ( n > bufCap ) { buf = (int16_t*)realloc( buf, n * 4 ); bufCap = n; }
				for ( int i = 0; i < n; i++ )
					for ( int c = 0; c < 2; c++ )
					{
						float v = Sample( frm, ( c < ch ) ? c : 0, i );
						v = v > 1.0f ? 1.0f : ( v < -1.0f ? -1.0f : v );
						buf[i*2+c] = (int16_t)( v * 32767.0f );
					}
				bufLen = n; bufPos = 0;
				av_frame_unref( frm );
				return true;
			}
			if ( r != AVERROR(EAGAIN) ) { eof = true; break; }
			if ( av_read_frame( fmt, pkt ) < 0 ) { eof = true; break; }
			if ( pkt->stream_index == stream )
				avcodec_send_packet( ctx, pkt );
			av_packet_unref( pkt );
		}
		return false;
	}

	static void SDLCALL Callback( void *ud, Uint8 *out, int len )
	{
		BinkFSAudio *a = (BinkFSAudio*)ud;
		int16_t *dst = (int16_t*)out;
		int frames = len / 4;
		while ( frames > 0 )
		{
			if ( a->bufPos >= a->bufLen && !a->Decode() )
			{
				memset( dst, 0, frames * 4 );
				return;
			}
			int n = MIN( frames, a->bufLen - a->bufPos );
			memcpy( dst, a->buf + a->bufPos * 2, n * 4 );
			dst += n * 2; a->bufPos += n; frames -= n;
		}
	}

	bool Open( const char *file )
	{
		if ( avformat_open_input( &fmt, file, nullptr, nullptr ) < 0 ) return false;
		if ( avformat_find_stream_info( fmt, nullptr ) < 0 ) return false;
		const AVCodec *codec = nullptr;
		stream = av_find_best_stream( fmt, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0 );
		if ( stream < 0 || !codec ) return false;
		ctx = avcodec_alloc_context3( codec );
		if ( !ctx || avcodec_parameters_to_context( ctx, fmt->streams[stream]->codecpar ) < 0 ) return false;
		if ( avcodec_open2( ctx, codec, nullptr ) < 0 ) return false;
		pkt = av_packet_alloc();
		frm = av_frame_alloc();

		SDL_AudioSpec want, have;
		SDL_zero( want );
		want.freq = ctx->sample_rate > 0 ? ctx->sample_rate : 44100;
		want.format = AUDIO_S16SYS;
		want.channels = 2;
		want.samples = 2048;
		want.callback = &BinkFSAudio::Callback;
		want.userdata = this;
		dev = SDL_OpenAudioDevice( nullptr, 0, &want, &have, 0 );
		if ( !dev ) return false;
		SDL_PauseAudioDevice( dev, 0 );
		return true;
	}

	void Close()
	{
		if ( dev ) { SDL_CloseAudioDevice( dev ); dev = 0; }
		if ( ctx ) avcodec_free_context( &ctx );
		if ( fmt ) avformat_close_input( &fmt );
		if ( pkt ) av_packet_free( &pkt );
		if ( frm ) av_frame_free( &frm );
		free( buf ); buf = nullptr;
	}
};

// ===========================================================================
// IVideoSubSystem Video Playback and Recording Services
// ===========================================================================
VideoResult_t CBinkVideoSubSystem::PlayVideoFileFullScreen( const char *filename, void *mainWindow, int windowWidth, int windowHeight, int desktopWidth, int desktopHeight, bool windowed, float forcedMinTime, VideoPlaybackFlags_t playbackFlags )
{
	AssertExitV( m_CurrentStatus == VideoSystemStatus::OK, SetResult( VideoResult::SYSTEM_NOT_AVAILABLE ) );
	AssertExitV( IS_NOT_EMPTY( filename ), SetResult( VideoResult::BAD_INPUT_PARAMETERS ) );

	CBinkMaterial *pMovie = new CBinkMaterial();
	if ( !pMovie->Init( "bink_fullscreen", filename, VideoPlaybackFlags::NO_PLAYBACK_OPTIONS ) )
	{
		delete pMovie;
		return SetResult( VideoResult::VIDEO_ERROR_OCCURED );
	}

	int imgW = 0, imgH = 0;
	float maxU = 1.0f, maxV = 1.0f;
	pMovie->GetVideoImageSize( &imgW, &imgH );
	pMovie->GetVideoTexCoordRange( &maxU, &maxV );
	if ( imgW <= 0 || imgH <= 0 || maxU <= 0.0f || maxV <= 0.0f )
	{
		pMovie->Shutdown();
		delete pMovie;
		return SetResult( VideoResult::VIDEO_ERROR_OCCURED );
	}
	const float texW = imgW / maxU, texH = imgH / maxV;

	// Init() already auto-starts playback

	// Letterbox the movie into the window
	int scrW = windowWidth > 0 ? windowWidth : desktopWidth;
	int scrH = windowHeight > 0 ? windowHeight : desktopHeight;
	float scale = MIN( (float)scrW / imgW, (float)scrH / imgH );
	int dw = (int)( imgW * scale ), dh = (int)( imgH * scale );
	int dx = ( scrW - dw ) / 2, dy = ( scrH - dh ) / 2;

	BinkFSAudio audio;
	if ( !audio.Open( filename ) )
		audio.Close();

	const double startTime = Plat_FloatTime();
	bool bAbort = false;

	while ( !bAbort )
	{
		SDL_Event ev;
		while ( SDL_PollEvent( &ev ) )
		{
			switch ( ev.type )
			{
			case SDL_QUIT:
			case SDL_KEYDOWN:
			case SDL_MOUSEBUTTONDOWN:
			case SDL_FINGERDOWN:
			case SDL_CONTROLLERBUTTONDOWN:
				bAbort = true;
				break;
			}
		}

		bool bPlaying = pMovie->Update();
		if ( !bPlaying )
			break;

		materials->BeginFrame( 0 );
		{
			CMatRenderContextPtr pRenderContext( materials );
			pRenderContext->Viewport( 0, 0, scrW, scrH );
			pRenderContext->ClearColor3ub( 0, 0, 0 );
			pRenderContext->ClearBuffers( true, true, true );
			pRenderContext->DrawScreenSpaceRectangle( pMovie->GetMaterial(), dx, dy, dw, dh,
				0, 0, imgW - 1, imgH - 1, (int)texW, (int)texH );
		}
		materials->EndFrame();
		materials->SwapBuffers();

		ThreadSleep( 2 );
	}

	// honour a forced minimum on-screen time
	if ( !bAbort && forcedMinTime > 0.0f )
	{
		while ( Plat_FloatTime() - startTime < forcedMinTime )
			ThreadSleep( 10 );
	}

	audio.Close();
	pMovie->StopVideo();
	pMovie->Shutdown();
	delete pMovie;
	return SetResult( VideoResult::SUCCESS );
}


// ===========================================================================
// IVideoSubSystem Video Material Services
//   note that the filename is absolute and has already resolved any paths
// ===========================================================================
IVideoMaterial* CBinkVideoSubSystem::CreateVideoMaterial( const char *pMaterialName, const char *pVideoFileName, VideoPlaybackFlags_t flags )
{
	SetResult( VideoResult::BAD_INPUT_PARAMETERS );
	AssertExitN( m_CurrentStatus == VideoSystemStatus::OK && IS_NOT_EMPTY( pMaterialName ) || IS_NOT_EMPTY( pVideoFileName ) );

	CBinkMaterial *pVideoMaterial = new CBinkMaterial();
	if ( pVideoMaterial == nullptr || pVideoMaterial->Init( pMaterialName, pVideoFileName, flags ) == false )
	{
		SAFE_DELETE( pVideoMaterial );
		SetResult( VideoResult::VIDEO_ERROR_OCCURED );
		return nullptr;
	}

	IVideoMaterial	*pInterface = (IVideoMaterial*) pVideoMaterial;
	m_MaterialList.AddToTail( pInterface );

	SetResult( VideoResult::SUCCESS );
	return pInterface;
}


VideoResult_t CBinkVideoSubSystem::DestroyVideoMaterial( IVideoMaterial *pVideoMaterial )
{
	AssertExitV( m_CurrentStatus == VideoSystemStatus::OK, SetResult( VideoResult::SYSTEM_NOT_AVAILABLE ) );
	AssertPtrExitV( pVideoMaterial, SetResult( VideoResult::BAD_INPUT_PARAMETERS ) );

	if ( m_MaterialList.Find( pVideoMaterial ) != -1 )
	{
		CBinkMaterial *pObject = (CBinkMaterial*) pVideoMaterial;
		pObject->Shutdown();
		delete pObject;

		m_MaterialList.FindAndFastRemove( pVideoMaterial );

		return SetResult( VideoResult::SUCCESS );
	}

	return SetResult (VideoResult::MATERIAL_NOT_FOUND );
}


// ===========================================================================
// IVideoSubSystem Video Recorder Services
// ===========================================================================
IVideoRecorder* CBinkVideoSubSystem::CreateVideoRecorder()
{
	SetResult( VideoResult::FEATURE_NOT_AVAILABLE );
	return nullptr;
}


VideoResult_t CBinkVideoSubSystem::DestroyVideoRecorder( IVideoRecorder *pRecorder )
{
	return SetResult( VideoResult::FEATURE_NOT_AVAILABLE );
}

VideoResult_t CBinkVideoSubSystem::CheckCodecAvailability( VideoEncodeCodec_t codec )
{
	AssertExitV( m_CurrentStatus == VideoSystemStatus::OK, SetResult( VideoResult::SYSTEM_NOT_AVAILABLE ) );
	AssertExitV( codec >= VideoEncodeCodec::DEFAULT_CODEC && codec < VideoEncodeCodec::CODEC_COUNT, SetResult( VideoResult::BAD_INPUT_PARAMETERS ) );

	return SetResult( VideoResult::FEATURE_NOT_AVAILABLE );
}


// ===========================================================================
// Status support
// ===========================================================================
VideoResult_t CBinkVideoSubSystem::GetLastResult()
{
	return m_LastResult;
}


VideoResult_t CBinkVideoSubSystem::SetResult( VideoResult_t status )
{
	m_LastResult = status;
	return status;
}


// ===========================================================================
// Bink Initialization & Shutdown
// ===========================================================================
bool CBinkVideoSubSystem::SetupBink()
{
	SetResult( VideoResult::INITIALIZATION_ERROR_OCCURED);
	AssertExitF( m_bBinkInitialized == false );

	// This is set early to indicate we have already been through here, even if we error out for some reason
	m_bBinkInitialized = true;
	m_CurrentStatus = VideoSystemStatus::OK;
	m_AvailableFeatures = DEFAULT_FEATURE_SET;
	// $$INIT CODE HERE$$


	// Note that we are now open for business....	
	m_bBinkInitialized = true;
	SetResult( VideoResult::SUCCESS );

	return true;
}


bool CBinkVideoSubSystem::ShutdownBink()
{
	if ( m_bBinkInitialized && m_CurrentStatus == VideoSystemStatus::OK )
	{

	}

	m_bBinkInitialized = false;
	m_CurrentStatus = VideoSystemStatus::NOT_INITIALIZED;
	m_AvailableFeatures = VideoSystemFeature::NO_FEATURES;
	SetResult( VideoResult::SUCCESS );

	return true;
}

