#include <ruby.h>
#include <stdio.h>

#include "core/macros.h"
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
#include "graphics/shader.h"
#include "audio/effect_processor.h"
#include "audio/audio_enums.h"
#include "audio/sound_source_cone.h"
#include "audio/listener.h"
#include "audio/sound_buffer.h"
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

static VALUE rb_mExt;

void Init_sfml_ext(void) {
    rb_mExt = rb_define_module("SFML");

    Init_Vector2(rb_mExt);
    Init_Vector3(rb_mExt);
    Init_Time(rb_mExt);
    Init_Sleep(rb_mExt);
    Init_Buffer(rb_mExt);
    Init_InputStream(rb_mExt);
    Init_Color(rb_mExt);
    Init_Rect(rb_mExt);
    Init_BlendMode(rb_mExt);
    Init_StencilMode(rb_mExt);
    Init_Image(rb_mExt);
    Init_Texture(rb_mExt);
    Init_Drawable(rb_mExt);
    Init_Transform(rb_mExt);
    Init_Transformable(rb_mExt);
    Init_Clock(rb_mExt);
    Init_Target(rb_mExt);
    Init_RenderState(rb_mExt);
    Init_Circle(rb_mExt);
    Init_RectangleShape(rb_mExt);
    Init_ConvexShape(rb_mExt);
    Init_Shape(rb_mExt);
    Init_Sprite(rb_mExt);
    Init_Vertex(rb_mExt);
    Init_VertexArray(rb_mExt);
    Init_VertexBuffer(rb_mExt);
    Init_Glyph(rb_mExt);
    Init_Font(rb_mExt);
    Init_Text(rb_mExt);
    Init_RenderTexture(rb_mExt);
    Init_Shader(rb_mExt);
    Init_Event(rb_mExt);
    Init_VideoMode(rb_mExt);
    Init_View(rb_mExt);
    Init_Window(rb_mExt);
    Init_Keyboard(rb_mExt);
    Init_Mouse(rb_mExt);
    Init_Joystick(rb_mExt);
    Init_Touch(rb_mExt);
    Init_Sensor(rb_mExt);
    Init_Clipboard(rb_mExt);
    Init_Cursor(rb_mExt);
    Init_ContextSettings(rb_mExt);
    Init_Context(rb_mExt);
    Init_Vulkan(rb_mExt);

    Init_EffectProcessor();
    Init_AudioEnums(rb_mExt);
    Init_SoundSourceCone(rb_mExt);
    Init_Listener(rb_mExt);
    Init_SoundBuffer(rb_mExt);
    Init_Sound(rb_mExt);
    Init_Music(rb_mExt);
    Init_SoundStream(rb_mExt);
    Init_SoundBufferRecorder(rb_mExt);
    Init_SoundRecorder(rb_mExt);

    Init_NetworkEnums(rb_mExt);
    Init_IpAddress(rb_mExt);
    Init_Packet(rb_mExt);
    Init_TcpSocket(rb_mExt);
    Init_TcpListener(rb_mExt);
    Init_UdpSocket(rb_mExt);
    Init_SocketSelector(rb_mExt);
    Init_Http(rb_mExt);
    Init_Ftp(rb_mExt);
}
