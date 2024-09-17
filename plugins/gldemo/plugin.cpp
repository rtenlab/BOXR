// clang-format off
#include <GL/glew.h> // GLEW has to be loaded before other GL libraries
// clang-format on

#include "illixr/data_format.hpp"
#include "illixr/error_util.hpp"
#include "illixr/extended_window.hpp"
#include "illixr/gl_util/obj.hpp"
#include "illixr/global_module_defs.hpp"
#include "illixr/math_util.hpp"
#include "illixr/phonebook.hpp"
#include "illixr/pose_prediction.hpp"
#include "illixr/shader_util.hpp"
#include "illixr/shaders/demo_shader.hpp"
#include "illixr/switchboard.hpp"
#include "illixr/threadloop.hpp"

#include <array>
#include <chrono>
#include <cmath>
#include <eigen3/Eigen/Core>
#include <future>
#include <iostream>
#include <thread>

using namespace ILLIXR;

// Wake up 1 ms after vsync instead of exactly at vsync to account for scheduling uncertainty
static constexpr std::chrono::milliseconds VSYNC_SAFETY_DELAY{1};

float ILLIXR::fr_r = 0.0f;

class gldemo : public threadloop {
public:
    // Public constructor, create_component passes Switchboard handles ("plugs")
    // to this constructor. In turn, the constructor fills in the private
    // references to the switchboard plugs, so the component can read the
    // data whenever it needs to.

    gldemo(const std::string& name_, phonebook* pb_)
        : threadloop{name_, pb_}
        , xwin{new xlib_gl_extended_window{1, 1, pb->lookup_impl<xlib_gl_extended_window>()->glc}}
        , sb{pb->lookup_impl<switchboard>()}
        , pp{pb->lookup_impl<pose_prediction>()}
        , _m_clock{pb->lookup_impl<RelativeClock>()}
        , _m_vsync{sb->get_reader<switchboard::event_wrapper<time_point>>("vsync_estimate")}
        , _m_image_handle{sb->get_writer<image_handle>("image_handle")}
        , _m_eyebuffer{sb->get_writer<rendered_frame>("eyebuffer")}
        , _m_signal_to_gldemo_finished{sb->get_writer<signal_to_gldemo_finished>("signal_to_gldemo_finished")} {
        spdlogger(std::getenv("GLDEMO_LOG_LEVEL"));
    }

    float calculate_fr_r(int n){
        float fr_r = 1.0f;
        if (n < num_init) {
            return fr_r;
        }
        float numerator = alpha_value * slope_C;
        float denominator = alpha_value * slope_C + (1 - alpha_value) * slope_M + (1/slope_K) * (n-num_init);
        fr_r = numerator / denominator;
        return fr_r;
    }

    int calculate_visible_vertices(Eigen::Matrix4f modelViewMatrix) {
    int visibleVertices = 0;
    for (const auto& object : demoscene.objects) {
        if (object.num_triangles > 0) {
            glBindBuffer(GL_ARRAY_BUFFER, object.vbo_handle);
            vertex_t* verts = (vertex_t*) glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
            if (verts != nullptr) {
                // int numVertices = object.num_triangles * 3; // Assuming each triangle has 3 vertices
                for (int i = 0; i < object.num_triangles; i+=10000) {
                    // Transform the vertex by the model-view-projection matrix
                    Eigen::Vector4f clipSpaceVertex = modelViewMatrix * Eigen::Vector4f(verts[i].position[0], verts[i].position[1], verts[i].position[2], 1.0f);

                    // Perform perspective division
                    Eigen::Vector3f ndcSpaceVertex = clipSpaceVertex.hnormalized();

                    // Check if the vertex is within the viewport
                    if (ndcSpaceVertex.x() >= -1.0f && ndcSpaceVertex.x() <= 1.0f &&
                        ndcSpaceVertex.y() >= -1.0f && ndcSpaceVertex.y() <= 1.0f) {
                        visibleVertices++;
                    }
                }
                glUnmapBuffer(GL_ARRAY_BUFFER);
            }
        }
    }
    return visibleVertices;
}

    // Essentially, a crude equivalent of XRWaitFrame.
    void wait_vsync() {
        switchboard::ptr<const switchboard::event_wrapper<time_point>> next_vsync = _m_vsync.get_ro_nullable();
        time_point                                                     now        = _m_clock->now();
        time_point                                                     wait_time{};

        if (next_vsync == nullptr) {
            // If no vsync data available, just sleep for roughly a vsync period.
            // We'll get synced back up later.
            std::this_thread::sleep_for(display_params::period);
            return;
        }

#ifndef NDEBUG
        if (log_count >= LOG_PERIOD) {
            double vsync_in = duration2double<std::milli>(**next_vsync - now);
            spdlog::get(name)->debug("First vsync is in {} ms", vsync_in);
        }
#endif

        bool hasRenderedThisInterval = (now - lastTime) < display_params::period;

        // If less than one frame interval has passed since we last rendered...
        if (hasRenderedThisInterval) {
            // We'll wait until the next vsync, plus a small delay time.
            // Delay time helps with some inaccuracies in scheduling.
            wait_time = **next_vsync + VSYNC_SAFETY_DELAY;

            // If our sleep target is in the past, bump it forward
            // by a vsync period, so it's always in the future.
            while (wait_time < now) {
                wait_time += display_params::period;
            }

#ifndef NDEBUG
            if (log_count >= LOG_PERIOD) {
                double wait_in = duration2double<std::milli>(wait_time - now);
                spdlog::get(name)->debug("Waiting until next vsync, in {} ms", wait_in);
            }
#endif
            // Perform the sleep.
            // TODO: Consider using Monado-style sleeping, where we nanosleep for
            // most of the wait, and then spin-wait for the rest?
            std::this_thread::sleep_for(wait_time - now);
        } else {
#ifndef NDEBUG
            if (log_count >= LOG_PERIOD) {
                spdlog::get(name)->debug("We haven't rendered yet, rendering immediately");
            }
#endif
        }
    }

    void _p_thread_setup() override {
        lastTime = _m_clock->now();
        _m_signal_to_gldemo_finished.put(_m_signal_to_gldemo_finished.allocate<signal_to_gldemo_finished>(signal_to_gldemo_finished{true}));

        // Note: glXMakeContextCurrent must be called from the thread which will be using it.
        [[maybe_unused]] const bool gl_result = static_cast<bool>(glXMakeCurrent(xwin->dpy, xwin->win, xwin->glc));
        assert(gl_result && "glXMakeCurrent should not fail");

// <RTEN> Initialize the parameters
        fr_r = 1.0f;
        alpha_value = 0.5;
        num_init = 3;
        slope_C = 3.7547;   
        slope_M = 0.1903;  
        slope_K = 3;
        n = 3;
// <RTEN/>           
    }

    void _p_one_iteration() override {
        // Essentially, XRWaitFrame.
        spdlog::get(name)->debug("<RTEN> gldemo begin before vsync: {}", 
                                duration2double<std::milli>(_m_clock->now().time_since_epoch()));
        wait_vsync();
#ifndef NDEBUG
        // <RTEN>
        [[maybe_unused]] time_point time_before_render = _m_clock->now();
        spdlog::get(name)->debug("<RTEN> gldemo begin after vsync: {}", 
                                duration2double<std::milli>(_m_clock->now().time_since_epoch()));
        _m_signal_to_gldemo_finished.put(_m_signal_to_gldemo_finished.allocate<signal_to_gldemo_finished>(signal_to_gldemo_finished{false}));
        // <RTEN/>
#endif
        glUseProgram(demoShaderProgram);
        glBindFramebuffer(GL_FRAMEBUFFER, eyeTextureFBO);

        glUseProgram(demoShaderProgram);
        glBindVertexArray(demo_vao);
        // glViewport(0, 0, display_params::width_pixels, display_params::height_pixels);

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

        glClearDepth(1);

        Eigen::Matrix4f modelMatrix = Eigen::Matrix4f::Identity();

        const fast_pose_type fast_pose = pp->get_fast_pose();
        pose_type            pose      = fast_pose.pose;

        Eigen::Matrix3f head_rotation_matrix = pose.orientation.toRotationMatrix();

        // Excessive? Maybe.
        constexpr int LEFT_EYE = 0;

        double     time_overhead = 0.0;
        double     time_srr = 0.0;

        [[maybe_unused]] time_point time_before_srr = _m_clock->now();
        for (auto eye_idx = 0; eye_idx < 2; eye_idx++) {

            // Offset of eyeball from pose
            auto eyeball =
                Eigen::Vector3f((eye_idx == LEFT_EYE ? -display_params::ipd / 2.0f : display_params::ipd / 2.0f), 0, 0);

            // Apply head rotation to eyeball offset vector
            eyeball = head_rotation_matrix * eyeball;

            // Apply head position to eyeball
            eyeball += pose.position;

            // Build our eye matrix from the pose's position + orientation.
            Eigen::Matrix4f eye_matrix   = Eigen::Matrix4f::Identity();
            eye_matrix.block<3, 1>(0, 3) = eyeball; // Set position to eyeball's position
            eye_matrix.block<3, 3>(0, 0) = pose.orientation.toRotationMatrix();

            // Objects' "view matrix" is inverse of eye matrix.
            auto view_matrix = eye_matrix.inverse();

            // We'll calculate this model view matrix
            // using fresh pose data, if we have any.
            Eigen::Matrix4f modelViewMatrix = view_matrix * modelMatrix;
            glUniformMatrix4fv(static_cast<GLint>(modelViewAttr), 1, GL_FALSE, (GLfloat*) (modelViewMatrix.data()));

            
// <RTEN>
            [[maybe_unused]] time_point overhead_start = _m_clock->now();

            Eigen::Vector3f centroid = demoscene.calculateCentroid();

            n = calculate_visible_vertices(modelViewMatrix);
            fr_r = calculate_fr_r(n);
            // Calculate the dimensions of the central region
            int centralWidth = display_params::width_pixels * fr_r;
            int centralHeight = display_params::height_pixels * fr_r;

            // Calculate the lower-left corner of the central region
            int x = (display_params::width_pixels - centralWidth) * fr_r;
            int y = (display_params::height_pixels - centralHeight) * fr_r;

            // Set the viewport to the central region
            glViewport(x, y, centralWidth, centralHeight);

            // Adjust the projection matrix to match the new viewport
            Eigen::Matrix4f centralProjection = basicProjection;
            centralProjection(0, 0) /= fr_r; // Scale the x-axis
            centralProjection(1, 1) /= fr_r; // Scale the y-axis

            time_overhead += duration2double<std::milli>(_m_clock->now() - overhead_start);
// </RTEN>
            
            // glUniformMatrix4fv(static_cast<GLint>(projectionAttr), 1, GL_FALSE, (GLfloat*) (basicProjection.data()));
            glUniformMatrix4fv(static_cast<GLint>(projectionAttr), 1, GL_FALSE, (GLfloat*) (centralProjection.data())); // <RTEN>

            glBindTexture(GL_TEXTURE_2D, eyeTextures[eye_idx]);
            glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, eyeTextures[eye_idx], 0);
            glBindTexture(GL_TEXTURE_2D, 0);
            glClearColor(0.9f, 0.9f, 0.9f, 1.0f);

            RAC_ERRNO_MSG("gldemo before glClear");
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // <RTEN> this line unlocks the frame
            RAC_ERRNO_MSG("gldemo after glClear");

            demoscene.Draw();
            

        }

        glFinish(); // <RTEN> this line locks the frame
        time_srr += duration2double<std::milli>(_m_clock->now() - time_before_srr);

#ifndef NDEBUG
        const double frame_duration_s = duration2double(_m_clock->now() - lastTime);
        const double fps              = 1.0 / frame_duration_s;

        if (log_count >= LOG_PERIOD) {
            spdlog::get(name)->debug("Submitting frame to buffer {}, frametime: {}, FPS: {}", which_buffer, frame_duration_s,
                                     fps);
            // <RTEN>
            spdlog::get(name)->debug("<RTEN> submitting time: {}", 
                                duration2double<std::milli>(_m_clock->now().time_since_epoch()));
            // <RTEN/>
        }
#endif
        lastTime = _m_clock->now();

        /// Publish our submitted frame handle to Switchboard!
        _m_eyebuffer.put(_m_eyebuffer.allocate<rendered_frame>(rendered_frame{
            // Somehow, C++ won't let me construct this object if I remove the `rendered_frame{` and `}`.
            // `allocate<rendered_frame>(...)` _should_ forward the arguments to rendered_frame's constructor, but I guess
            // not.
            std::array<GLuint, 2>{0, 0}, std::array<GLuint, 2>{which_buffer, which_buffer}, fast_pose,
            fast_pose.predict_computed_time, lastTime}));

        which_buffer = !which_buffer;

        // Signal to the main thread that we're done rendering
        _m_signal_to_gldemo_finished.put(_m_signal_to_gldemo_finished.allocate<signal_to_gldemo_finished>(signal_to_gldemo_finished{true}));

#ifndef NDEBUG
        if (log_count >= LOG_PERIOD) {
            log_count = 0;
        } else {
            log_count++;
        }
        // <RTEN>
        [[maybe_unused]] time_point time_after_render = _m_clock->now();
        double     time_duration         = duration2double<std::milli>(time_after_render - time_before_render);
        spdlog::get(name)->debug("<RTEN> gldemo render time: {} ms", time_duration);

        // double c2d_lat = duration2double<std::milli>(time_after_render - begin_c2d_tp);
        // spdlog::get(name)->debug("<RTEN> c2d: {} ms", c2d_lat);

        // print demoscene objects number
        spdlog::get(name)->debug("<RTEN> demoscene objects number: {}", demoscene.objects.size());

        // print the viewport number of vertices
        spdlog::get(name)->debug("<RTEN> number of visible vertices: {}", n);

        // print fr_r
        spdlog::get(name)->debug("<RTEN> fr_r: {}", fr_r);

        // spdlog::get(name)->debug("<RTEN> c2d end: {}", );

        // print the time overhead
        spdlog::get(name)->debug("<RTEN> time overhead: {} ms", time_overhead);

        // print the time spent in SRR
        spdlog::get(name)->debug("<RTEN> time spent in SRR: {} ms", time_srr);
        // <RTEN/>
#endif
    }

#ifndef NDEBUG
    size_t log_count  = 0;
    size_t LOG_PERIOD = 0; // <RTEN> log every output frame
#endif

private:
    const std::unique_ptr<const xlib_gl_extended_window>              xwin;
    const std::shared_ptr<switchboard>                                sb;
    const std::shared_ptr<pose_prediction>                            pp;
    const std::shared_ptr<const RelativeClock>                        _m_clock;
    const switchboard::reader<switchboard::event_wrapper<time_point>> _m_vsync;

    // Switchboard plug for application eye buffer.
    // We're not "writing" the actual buffer data,
    // we're just atomically writing the handle to the
    // correct eye/framebuffer in the "swapchain".
    switchboard::writer<image_handle>   _m_image_handle;
    switchboard::writer<rendered_frame> _m_eyebuffer;
    switchboard::writer<signal_to_gldemo_finished> _m_signal_to_gldemo_finished;

    GLuint eyeTextures[2]{};
    GLuint eyeTextureFBO{};
    GLuint eyeTextureDepthTarget{};

    unsigned char which_buffer = 0;

    GLuint demo_vao{};
    GLuint demoShaderProgram{};

    GLuint vertexPosAttr{};
    GLuint vertexNormalAttr{};
    GLuint modelViewAttr{};
    GLuint projectionAttr{};

    GLuint colorUniform{};

    ObjScene demoscene;

    Eigen::Matrix4f basicProjection;

    time_point lastTime{};

    float alpha_value = 0.0;      // Example trade-off weight
    int num_init = 0;             // Initial number of vertices
    float slope_C = 0.0;          // Derived coefficient for time improvement
    float slope_M = 0.0;          // Derived coefficient for quality degradation
    float slope_K = 0.0;          // Example scaling factor for number of vertices
    int n = 0;                    // Current number of vertices

    static void createSharedEyebuffer(GLuint* texture_handle) {
        // Create the shared eye texture handle
        glGenTextures(1, texture_handle);
        glBindTexture(GL_TEXTURE_2D, *texture_handle);

        // Set the texture parameters for the texture that the FBO will be mapped into
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, display_params::width_pixels, display_params::height_pixels, 0, GL_RGB,
                     GL_UNSIGNED_BYTE, nullptr);

        // Unbind texture
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void createFBO(const GLuint* texture_handle, GLuint* fbo, GLuint* depth_target) {
        // Create a framebuffer to draw some things to the eye texture
        glGenFramebuffers(1, fbo);

        // Bind the FBO as the active framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, *fbo);
        glGenRenderbuffers(1, depth_target);
        glBindRenderbuffer(GL_RENDERBUFFER, *depth_target);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, display_params::width_pixels, display_params::height_pixels);
        // glRenderbufferStorageMultisample(GL_RENDERBUFFER, fboSampleCount, GL_DEPTH_COMPONENT, display_params::width_pixels,
        // display_params::height_pixels);

        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        // Bind eyebuffer texture
        spdlog::get(name)->info("About to bind eyebuffer texture, texture handle: {}", *texture_handle);

        glBindTexture(GL_TEXTURE_2D, *texture_handle);
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, *texture_handle, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        // attach a renderbuffer to depth attachment point
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, *depth_target);

        // Unbind FBO
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

public:
    // We override start() to control our own lifecycle
    void start() override {
        [[maybe_unused]] const bool gl_result_0 = static_cast<bool>(glXMakeCurrent(xwin->dpy, xwin->win, xwin->glc));
        assert(gl_result_0 && "glXMakeCurrent should not fail");

        // Init and verify GLEW
        const GLenum glew_err = glewInit();
        if (glew_err != GLEW_OK) {
            spdlog::get(name)->error("GLEW Error: {}", glewGetErrorString(glew_err));
            ILLIXR::abort("Failed to initialize GLEW");
        }

        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(MessageCallback, nullptr);

        // Create two shared textures, one for each eye.
        createSharedEyebuffer(&(eyeTextures[0]));
        _m_image_handle.put(
            _m_image_handle.allocate<image_handle>(image_handle{eyeTextures[0], 1, swapchain_usage::LEFT_SWAPCHAIN}));
        createSharedEyebuffer(&(eyeTextures[1]));
        _m_image_handle.put(
            _m_image_handle.allocate<image_handle>(image_handle{eyeTextures[1], 1, swapchain_usage::RIGHT_SWAPCHAIN}));

        // Initialize FBO and depth targets, attaching to the frame handle
        createFBO(&(eyeTextures[0]), &eyeTextureFBO, &eyeTextureDepthTarget);

        // Create and bind global VAO object
        glGenVertexArrays(1, &demo_vao);
        glBindVertexArray(demo_vao);

        demoShaderProgram = init_and_link(demo_vertex_shader, demo_fragment_shader);
#ifndef NDEBUG
        spdlog::get(name)->debug("Demo app shader program is program {}", demoShaderProgram);
#endif

        vertexPosAttr    = glGetAttribLocation(demoShaderProgram, "vertexPosition");
        vertexNormalAttr = glGetAttribLocation(demoShaderProgram, "vertexNormal");
        modelViewAttr    = glGetUniformLocation(demoShaderProgram, "u_modelview");
        projectionAttr   = glGetUniformLocation(demoShaderProgram, "u_projection");
        colorUniform     = glGetUniformLocation(demoShaderProgram, "u_color");

        // Load/initialize the demo scene
        char* obj_dir = std::getenv("ILLIXR_DEMO_DATA");
        if (obj_dir == nullptr) {
            ILLIXR::abort("Please define ILLIXR_DEMO_DATA.");
        }

        demoscene = ObjScene(std::string(obj_dir), "scene.obj");

        // Construct perspective projection matrix
        math_util::projection_fov(&basicProjection, display_params::fov_x / 2.0f, display_params::fov_x / 2.0f,
                                  display_params::fov_y / 2.0f, display_params::fov_y / 2.0f, rendering_params::near_z,
                                  rendering_params::far_z);

        [[maybe_unused]] const bool gl_result_1 = static_cast<bool>(glXMakeCurrent(xwin->dpy, None, nullptr));
        assert(gl_result_1 && "glXMakeCurrent should not fail");

        // Effectively, last vsync was at zero.
        // Try to run gldemo right away.
        threadloop::start();
    }
};

PLUGIN_MAIN(gldemo)
