#include <ruby.h>
#include <stdio.h>

#include "core/macros.h"
#include "core/foreign_thread.h"
#include "system/vec2.h"
#include "system/vec3.h"
#include "system/time.h"
#include "graphics/transform.h"
#include "graphics/drawable.h"
#include "graphics/color.h"
#include "graphics/rect.h"
#include "graphics/blend_mode.h"
#include "graphics/stencil_mode.h"
#include "graphics/image.h"
#include "graphics/texture.h"
#include "graphics/transformable.h"
#include "system/clock.h"
#include "system/sleep.h"
#include "system/buffer.h"
#include "system/input_stream.h"
#include "graphics/target.h"
#include "graphics/render_state.h"
#include "window/event.h"
#include "window/window_base.h"
#include "window/window.h"
#include "window/keyboard.h"
#include "window/mouse.h"
#include "window/joystick.h"
#include "window/touch.h"
#include "window/sensor.h"
#include "window/clipboard.h"
#include "window/cursor.h"
#include "window/context_settings.h"
#include "window/context.h"
#include "window/vulkan.h"
#include "graphics/view.h"
#include "window/video_mode.h"
#include "graphics/circle.h"
#include "graphics/rectangle.h"
#include "graphics/polygon.h"
#include "graphics/shape.h"
#include "graphics/sprite.h"
#include "graphics/vertex.h"
#include "graphics/vertex_array.h"
#include "graphics/vertex_buffer.h"
#include "graphics/glyph.h"
#include "graphics/font.h"
#include "graphics/text.h"
#include "graphics/render_texture.h"
#include "graphics/render_window.h"
#include "graphics/shader.h"
#include "audio/effect_processor.h"
#include "audio/audio_enums.h"
#include "audio/sound_source_cone.h"
#include "audio/listener.h"
#include "audio/sound_buffer.h"
#include "audio/sound_source.h"
#include "audio/sound.h"
#include "audio/music.h"
#include "audio/sound_stream.h"
#include "audio/sound_buffer_recorder.h"
#include "audio/sound_recorder.h"
#include "network/network_enums.h"
#include "network/ip_address.h"
#include "network/packet.h"
#include "network/tcp_socket.h"
#include "network/tcp_listener.h"
#include "network/udp_socket.h"
#include "network/socket_selector.h"
#include "network/http.h"
#include "network/ftp.h"

//  C Naming Convention:
//
//    Struct              TitleCase
//    Struct Members      lower_case or lowerCase
//
//    Enum                ETitleCase
//    Enum Members        ALL_CAPS or lowerCase
//
//    Public functions    pfx_TitleCase (pfx = two or three letter module prefix)
//    Private functions   TitleCase
//    Trivial variables   i,x,n,f etc...
//    Local variables     lower_case or lowerCase
//    Global variables    g_lowerCase or g_lower_case (searchable by g_ prefix)

static VALUE rb_mSF;

/* Document-module: SF
 * Ruby bindings for SFML 3, via its C API, CSFML. Every class and module in
 * this library lives under one of the subsystem modules of this namespace.
 */
void Init_sfml_ext(void) {
    VALUE rb_mSystem;
    VALUE rb_mGraphics;
    VALUE rb_mWindow;
    VALUE rb_mAudio;
    VALUE rb_mNetwork;

    rb_mSF = rb_define_module("SF");

    rb_mSystem = rb_define_module_under(rb_mSF, "System");
    rb_mGraphics = rb_define_module_under(rb_mSF, "Graphics");
    rb_mWindow = rb_define_module_under(rb_mSF, "Window");
    rb_mAudio = rb_define_module_under(rb_mSF, "Audio");
    rb_mNetwork = rb_define_module_under(rb_mSF, "Network");

    Init_ForeignThread();

    Init_Vector2(rb_mSystem);
    Init_Vector3(rb_mSystem);
    Init_Time(rb_mSystem);
    Init_Sleep(rb_mSystem);
    Init_Buffer(rb_mSystem);
    Init_InputStream(rb_mSystem);
    Init_Color(rb_mGraphics);
    Init_Rect(rb_mGraphics);
    Init_BlendMode(rb_mGraphics);
    Init_StencilMode(rb_mGraphics);
    Init_Image(rb_mGraphics);
    Init_Texture(rb_mGraphics);
    Init_Drawable(rb_mGraphics);
    Init_Transform(rb_mGraphics);
    Init_Transformable(rb_mGraphics);
    Init_Clock(rb_mSystem);
    Init_Target(rb_mGraphics);
    Init_RenderState(rb_mGraphics);
    Init_Shape(rb_mGraphics);
    Init_Circle(rb_mGraphics);
    Init_RectangleShape(rb_mGraphics);
    Init_ConvexShape(rb_mGraphics);
    Init_Sprite(rb_mGraphics);
    Init_Vertex(rb_mGraphics);
    Init_VertexArray(rb_mGraphics);
    Init_VertexBuffer(rb_mGraphics);
    Init_Glyph(rb_mGraphics);
    Init_Font(rb_mGraphics);
    Init_Text(rb_mGraphics);
    Init_RenderTexture(rb_mGraphics);
    Init_Shader(rb_mGraphics);
    Init_Event(rb_mWindow);
    Init_VideoMode(rb_mWindow);
    Init_View(rb_mGraphics);
    Init_WindowBase(rb_mWindow);
    Init_Window(rb_mWindow);
    Init_RenderWindow(rb_mGraphics);
    Init_Keyboard(rb_mWindow);
    Init_Mouse(rb_mWindow);
    Init_Joystick(rb_mWindow);
    Init_Touch(rb_mWindow);
    Init_Sensor(rb_mWindow);
    Init_Clipboard(rb_mWindow);
    Init_Cursor(rb_mWindow);
    Init_ContextSettings(rb_mWindow);
    Init_Context(rb_mWindow);
    Init_Vulkan(rb_mWindow);

    Init_EffectProcessor();
    Init_AudioEnums(rb_mAudio);
    Init_SoundSourceCone(rb_mAudio);
    Init_Listener(rb_mAudio);
    Init_SoundBuffer(rb_mAudio);
    Init_SoundSource(rb_mAudio);
    Init_Sound(rb_mAudio);
    Init_SoundStream(rb_mAudio);
    Init_Music(rb_mAudio);
    Init_SoundRecorder(rb_mAudio);
    Init_SoundBufferRecorder(rb_mAudio);

    Init_NetworkEnums(rb_mNetwork);
    Init_IpAddress(rb_mNetwork);
    Init_Packet(rb_mNetwork);
    Init_TcpSocket(rb_mNetwork);
    Init_TcpListener(rb_mNetwork);
    Init_UdpSocket(rb_mNetwork);
    Init_SocketSelector(rb_mNetwork);
    Init_Http(rb_mNetwork);
    Init_Ftp(rb_mNetwork);
}
