#include "MeshTest.h"
#include "MeshOperations.h"


#include "Magnum/GL/Version.h"
#include "Magnum/Platform/GLContext.h"


using namespace std;
using namespace Magnum;
using namespace Magnum::Trade;
using namespace Magnum::Primitives;
using namespace Magnum::Platform;
using namespace Math::Literals;


MeshTest::Configuration::Configuration():
    _title{"Mesh Test"},
    _size{700, 700}, _sampleCount{0},
    _version{GL::Version::GL410},
    _windowFlags{WindowFlag::Focused},
    _cursorMode{CursorMode::Normal},
    _srgbCapable{true}
{
}

MeshTest::Configuration::~Configuration() = default;

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    MeshTest *foo = (MeshTest*)glfwGetWindowUserPointer(window);
    foo->mouseMove(xpos, ypos);
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void window_refresh_callback(GLFWwindow* window)
{
    MeshTest *foo = (MeshTest*)glfwGetWindowUserPointer(window);
    foo->draw();
}

static void char_callback(GLFWwindow *window, unsigned int c) {
    ::setMeshOpDebugMode(c);
    window_refresh_callback(window);
}


// The callback function receives two-dimensional scroll offsets.
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    std::cout << "scroll offset : (" << xoffset << ", " << yoffset << ")" << std::endl;

    MeshTest *foo = (MeshTest*)glfwGetWindowUserPointer(window);

    foo->centerShift[2] -= 0.1 * yoffset;
    foo->draw();
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    MeshTest *foo = (MeshTest*)glfwGetWindowUserPointer(window);
    foo->mouseClick(button, action, mods);
}


HRESULT MeshTest::createContext(const Configuration& configuration) {
    CORRADE_ASSERT(context->version() ==
            GL::Version::None,
            "Platform::GlfwApplication::tryCreateContext(): context already created",
            false);

    /* Window flags */
    GLFWmonitor* monitor = nullptr; /* Needed for setting fullscreen */
    if (configuration.windowFlags() >= Configuration::WindowFlag::Fullscreen) {
        monitor = glfwGetPrimaryMonitor();
        glfwWindowHint(GLFW_AUTO_ICONIFY, configuration.windowFlags() >= Configuration::WindowFlag::AutoIconify);
    } else {
        const Configuration::WindowFlags& flags = configuration.windowFlags();

        glfwWindowHint(GLFW_VISIBLE, !(flags >= Configuration::WindowFlag::Hidden));
        #ifdef GLFW_MAXIMIZED
        glfwWindowHint(GLFW_MAXIMIZED, flags >= Configuration::WindowFlag::Maximized);
        #endif
        glfwWindowHint(GLFW_FLOATING, flags >= Configuration::WindowFlag::Floating);
    }

    glfwWindowHint(GLFW_RESIZABLE, true);
    glfwWindowHint(GLFW_FOCUSED, configuration.windowFlags() >= Configuration::WindowFlag::Focused);

    /* Context window hints */
    glfwWindowHint(GLFW_SAMPLES, configuration.sampleCount());
    glfwWindowHint(GLFW_SRGB_CAPABLE, configuration.isSRGBCapable());

    const Configuration::Flags& flags = configuration.flags();
    #ifdef GLFW_CONTEXT_NO_ERROR
    glfwWindowHint(GLFW_CONTEXT_NO_ERROR, flags >= Configuration::Flag::NoError);
    #endif
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, flags >= Configuration::Flag::Debug);
    glfwWindowHint(GLFW_STEREO, flags >= Configuration::Flag::Stereo);

    /* Set context version, if requested */
    if(configuration.version() != GL::Version::None) {
        Int major, minor;
        std::tie(major, minor) = version(configuration.version());

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);

        if(configuration.version() >= GL::Version::GL310) {
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        }
    }

    /* Set context flags */
    window = glfwCreateWindow(configuration.size().x(), configuration.size().y(),
                              configuration.title().c_str(), monitor, nullptr);
    if(!window) {
        Error() << "Platform::GlfwApplication::tryCreateContext(): cannot create context";
        glfwTerminate();
        return false;
    }

    glfwSetWindowUserPointer(window, this);

    glfwSetWindowPos(window, 500, 100);

    /* Proceed with configuring other stuff that couldn't be done with window
       hints */
    if(configuration.windowFlags() >= Configuration::WindowFlag::Minimized)
        glfwIconifyWindow(window);

    glfwSetInputMode(window, GLFW_CURSOR, Int(configuration.cursorMode()));

    /* Set callbacks */

    glfwSetWindowRefreshCallback(window, window_refresh_callback);

    glfwSetCursorPosCallback(window, cursor_position_callback);

    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glfwSetCharCallback(window, char_callback);

    glfwSetScrollCallback(window, scroll_callback);


    //glfwSetFramebufferSizeCallback(_window, staticViewportEvent);
    //glfwSetKeyCallback(_window, staticKeyEvent);
    //glfwSetCursorPosCallback(_window, staticMouseMoveEvent);
    //glfwSetMouseButtonCallback(_window, staticMouseEvent);
    //glfwSetScrollCallback(_window, staticMouseScrollEvent);
    //glfwSetCharCallback(_window, staticTextInputEvent);

    glfwMakeContextCurrent(window);

    /* Return true if the initialization succeeds */
    return context->tryCreate();
}


MeshTest::MeshTest(const Configuration& configuration) :
    context{new Magnum::Platform::GLContext{NoCreate, 0, nullptr}}
{
    /* Init GLFW */
    glfwSetErrorCallback(error_callback);

    if(!glfwInit()) {
        Error() << "Could not initialize GLFW";
        std::exit(8);
    }

    createContext(configuration);

    // need to enabler depth testing. The graphics processor can draw each facet in any order it wants.
    // Depth testing makes sure that front facing facts are drawn after back ones, so that back facets
    // don't cover up front ones.
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);

    // don't draw facets that face away from us. We have A LOT of these INSIDE cells, no need to
    // draw them.
    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable( GL_BLEND );

    renderer = new MxMeshRenderer{MxMeshRenderer::Flag::Wireframe};

    GL::Renderer::setClearColor(Color4{1.0f, 1.0f, 1.0f, 1.0f});

    loadModel();
}


MeshTest::MeshTest() : MeshTest{Configuration{}}
{
}

void MeshTest::step(float dt) {
    propagator->step(dt);
    draw();
}

void MeshTest::draw() {

    Vector3 min, max;
    std::tie(min, max) = model->mesh->extents();

    center = (max + min)/2;

    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color|GL::FramebufferClear::Depth);

    renderer->setViewportSize(Vector2{GL::defaultFramebuffer.viewport().size()});

    projection = Matrix4::perspectiveProjection(35.0_degf,
                                                Vector2{GL::defaultFramebuffer.viewport().size()}.aspectRatio(),
                    0.01f, 100.0f);


    renderer->setProjectionMatrix(projection);

    Matrix4 mat = Matrix4::translation(centerShift) * rotation * Matrix4::translation(-center) ;

    renderer->setViewMatrix(mat);

    renderer->setColor(Color4::yellow());

    renderer->setWireframeColor(Color4{0., 0., 0.});

    renderer->setWireframeWidth(2.0);

    renderer->draw();

    glfwSwapBuffers(window);
}

void MeshTest::mouseMove(double xpos, double ypos) {

    Vector2 pos{(float)xpos, (float)ypos};

    const Vector2 delta = 3.0f *
    Vector2{pos - previousMousePosition} /
    Vector2{GL::defaultFramebuffer.viewport().size()};

    previousMousePosition = pos;

    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {

    rotation =
        rotation *
        Matrix4::rotationX(Rad{delta.y()}) *
        Matrix4::rotationY(Rad{delta.x()}) ;
    }

    else if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) {

        centerShift = centerShift + Vector3{{delta.x(), -delta.y(), 0.f}};
    }

    draw();
}

void MeshTest::mouseClick(int button, int action, int mods) {

    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        previousMousePosition = Vector2{(float)xpos, (float)ypos};
    }
}


template <typename Type, typename Obj>
void testCast(Obj obj) {
    void *p = dyn_cast<Type>(obj);
    const char* name = Type::type()->tp_name;
    const char* is = (p) ? "true" : "false";
    std::cout << "as " <<  name << ": " << is << std::endl;
}


template <typename Type>
void testObject(Type *obj) {

    std::cout << "obj name: " << obj->ob_type->tp_name << std::endl;

    testCast<CObject>(obj);
    testCast<MxEdge>(obj);
    testCast<MxCell>(obj);
    testCast<MxPolygon>(obj);
    testCast<MxVertex>(obj);
}

void MeshTest::testMeshTypes()
{

    for(auto p : model->mesh->cells) {
        testObject(p);
    }

    for(auto p : model->mesh->edges) {
        testObject(p);
    }

    for(auto p : model->mesh->polygons) {
        testObject(p);

        testObject(&p->partialPolygons[0]);
        testObject(&p->partialPolygons[1]);
    }

    for(auto p : model->mesh->vertices) {
        testObject(p);
    }

    std::cout << "done testing" << std::endl;

}

void MeshTest::loadModel()
{
    delete model;
    delete propagator;

    model = new GrowthModel{};

    propagator = new LangevinPropagator{};

    VERIFY(MxBind_PropagatorModel(propagator, model));

    VERIFY(model->loadModel());

    Vector3 min, max;
    std::tie(min, max) = model->mesh->extents();

    center = (max + min)/2;

    renderer->setMesh(model->mesh);

    testMeshTypes();

    draw();
}
