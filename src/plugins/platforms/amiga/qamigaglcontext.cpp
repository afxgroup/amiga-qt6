#include "qamigaglcontext.h"

#include <QtCore/qdebug.h>

#include <stdlib.h>

QT_BEGIN_NAMESPACE

int QAmigaOpenGLContext::noContexts = 0;
struct Library *MesaBase;
struct MesaIFace *IMesa;

struct Screen *qt_intuition_screen = 0;

// Desktop profile request: core needs 3.2 or newer, everything else runs as
// compatibility, which is what mesa.library gives for an unversioned context.
static MesaProfile mesaProfileFor(const QSurfaceFormat &format)
{
    const bool coreVersion = format.majorVersion() > 3
                             || (format.majorVersion() == 3 && format.minorVersion() >= 2);
    if (format.profile() == QSurfaceFormat::CoreProfile && coreVersion)
        return MESA_PROFILE_CORE;
    return MESA_PROFILE_COMPATIBILITY;
}

QAmigaOpenGLContext::QAmigaOpenGLContext(QOpenGLContext *context)
    : QPlatformOpenGLContext(),
      mesaContext(nullptr),
      mesaDrawable(nullptr),
      drawableWindow(nullptr),
      share(nullptr),
      context(context),
      surface(nullptr),
      dummyW(nullptr)
{
    if (!MesaBase) {
        MesaBase = IExec->OpenLibrary("mesa.library", 1);
        if (MesaBase)
            IMesa = (struct MesaIFace *) IExec->GetInterface(MesaBase, "main", 1, NULL);
    }
    if (!MesaBase || !IMesa) {
        qFatal("mesa.library not found. OpenGL rendering is not possible on this platform.\n");
        exit(20);
    }
    noContexts++;

    requestedFormat = context->format();

    QOpenGLContext *shareContext = context->shareContext();
    QAmigaOpenGLContext *newShare = shareContext ? static_cast<QAmigaOpenGLContext *>(shareContext->handle()) : 0;
    share = newShare ? newShare->mesaContext : 0;

    dummyW = IIntuition->OpenWindowTags(NULL,
        WA_Title,				"",
        WA_SimpleRefresh,		TRUE,
        WA_InnerWidth,			640,
        WA_InnerHeight,			480,
        WA_BackFill, 			LAYERS_NOBACKFILL,
        WA_Hidden,              TRUE,
        TAG_DONE);
    if (!dummyW)
        return;

    // The context is created on a drawable, so the hidden window provides the
    // visual that every later drawable must match.
    if (createDrawable(dummyW))
        createContext();
}

QAmigaOpenGLContext::~QAmigaOpenGLContext()
{
    if (mesaContext) {
        IMesa->MesaDestroyContext(mesaContext);
        mesaContext = nullptr;
    }
    destroyDrawable();
    if (dummyW) {
        IIntuition->CloseWindow(dummyW);
        dummyW = nullptr;
    }
    if (--noContexts == 0 && MesaBase) {
        if (IMesa)
            IExec->DropInterface((struct Interface *)IMesa);
        IExec->CloseLibrary(MesaBase);
        MesaBase = nullptr;
        IMesa = nullptr;
    }
}

bool QAmigaOpenGLContext::createContext()
{
    struct TagItem contextTags[8];
    int count = 0;

    contextTags[count].ti_Tag  = MESA_CONTEXT_API;
    contextTags[count].ti_Data = MESA_API_OPENGL;
    count++;

    contextTags[count].ti_Tag  = MESA_CONTEXT_PROFILE;
    contextTags[count].ti_Data = mesaProfileFor(requestedFormat);
    count++;

    contextTags[count].ti_Tag  = MESA_CONTEXT_MAJOR_VERSION;
    contextTags[count].ti_Data = (ULONG) requestedFormat.majorVersion();
    count++;

    contextTags[count].ti_Tag  = MESA_CONTEXT_MINOR_VERSION;
    contextTags[count].ti_Data = (ULONG) requestedFormat.minorVersion();
    count++;

    if (share) {
        contextTags[count].ti_Tag  = MESA_CONTEXT_SHARE_WITH;
        contextTags[count].ti_Data = (ULONG) share;
        count++;
    }

    contextTags[count].ti_Tag  = TAG_DONE;
    contextTags[count].ti_Data = 0;

    const MesaStatus status = IMesa->MesaCreateContext(mesaDrawable, contextTags, &mesaContext);
    if (status != MESA_STATUS_OK) {
        qWarning("mesa.library: MesaCreateContext failed (status %d)", (int) status);
        mesaContext = nullptr;
        return false;
    }
    return true;
}

bool QAmigaOpenGLContext::createDrawable(struct Window *window)
{
    struct TagItem drawableTags[10];
    int count = 0;

    // A bit count of zero or less means "let mesa.library pick the window's
    // native format", so only the values the application asked for are passed.
    if (requestedFormat.redBufferSize() > 0) {
        drawableTags[count].ti_Tag  = MESA_DRAWABLE_RED_BITS;
        drawableTags[count].ti_Data = (ULONG) requestedFormat.redBufferSize();
        count++;
    }
    if (requestedFormat.greenBufferSize() > 0) {
        drawableTags[count].ti_Tag  = MESA_DRAWABLE_GREEN_BITS;
        drawableTags[count].ti_Data = (ULONG) requestedFormat.greenBufferSize();
        count++;
    }
    if (requestedFormat.blueBufferSize() > 0) {
        drawableTags[count].ti_Tag  = MESA_DRAWABLE_BLUE_BITS;
        drawableTags[count].ti_Data = (ULONG) requestedFormat.blueBufferSize();
        count++;
    }
    if (requestedFormat.alphaBufferSize() > 0) {
        drawableTags[count].ti_Tag  = MESA_DRAWABLE_ALPHA_BITS;
        drawableTags[count].ti_Data = (ULONG) requestedFormat.alphaBufferSize();
        count++;
    }

    // Depth and stencil fall back to 24 and 8 bits when the format leaves them unset.
    drawableTags[count].ti_Tag  = MESA_DRAWABLE_DEPTH_BITS;
    drawableTags[count].ti_Data = (ULONG) (requestedFormat.depthBufferSize() > 0 ? requestedFormat.depthBufferSize() : 24);
    count++;

    drawableTags[count].ti_Tag  = MESA_DRAWABLE_STENCIL_BITS;
    drawableTags[count].ti_Data = (ULONG) (requestedFormat.stencilBufferSize() > 0 ? requestedFormat.stencilBufferSize() : 8);
    count++;

    if (requestedFormat.samples() > 0) {
        drawableTags[count].ti_Tag  = MESA_DRAWABLE_SAMPLES;
        drawableTags[count].ti_Data = (ULONG) requestedFormat.samples();
        count++;
    }

    drawableTags[count].ti_Tag  = MESA_DRAWABLE_DOUBLE_BUFFERED;
    drawableTags[count].ti_Data = requestedFormat.swapBehavior() != QSurfaceFormat::SingleBuffer ? TRUE : FALSE;
    count++;

    drawableTags[count].ti_Tag  = TAG_DONE;
    drawableTags[count].ti_Data = 0;

    MesaDrawable drawable = nullptr;
    const MesaStatus status = IMesa->MesaCreateWindowDrawable(window, drawableTags, &drawable);
    if (status != MESA_STATUS_OK) {
        qWarning("mesa.library: MesaCreateWindowDrawable failed (status %d)", (int) status);
        return false;
    }

    mesaDrawable = drawable;
    drawableWindow = window;
    return true;
}

void QAmigaOpenGLContext::destroyDrawable()
{
    if (mesaDrawable) {
        IMesa->MesaDestroyDrawable(mesaDrawable);
        mesaDrawable = nullptr;
    }
    drawableWindow = nullptr;
}

QSurfaceFormat QAmigaOpenGLContext::format() const
{
    QSurfaceFormat format = requestedFormat;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setRedBufferSize(8);
    format.setGreenBufferSize(8);
    format.setBlueBufferSize(8);
    format.setAlphaBufferSize(8);
    return format;
}

void QAmigaOpenGLContext::swapBuffers(QPlatformSurface *platformSurface)
{
    if (!makeCurrent(platformSurface))
        return;

    const MesaStatus status = IMesa->MesaSwapBuffers(mesaDrawable);
    if (status != MESA_STATUS_OK)
        qWarning("mesa.library: MesaSwapBuffers failed (status %d)", (int) status);
}

bool QAmigaOpenGLContext::makeCurrent(QPlatformSurface *platformSurface)
{
    if (!mesaContext)
        return false;

    QAmigaWindow *amigaWindow = dynamic_cast<QAmigaWindow *>(platformSurface);
    QAmigaOffscreenSurface *offscreenSurface = dynamic_cast<QAmigaOffscreenSurface *>(platformSurface);

    surface = platformSurface;
    struct Window *window = amigaWindow ? amigaWindow->intuitionWindow() : (offscreenSurface ? offscreenSurface->nativeHandle() : 0);
    if (!window)
        window = dummyW;
    if (!window)
        return false;

    // Drawables are tied to the Intuition window they were created from, so a
    // surface on another window gets a new drawable before it becomes current.
    if (!mesaDrawable || window != drawableWindow) {
        IMesa->MesaUnbindCurrent();
        destroyDrawable();
        if (!createDrawable(window))
            return false;
    }

    const MesaStatus status = IMesa->MesaMakeCurrent(mesaContext, mesaDrawable, mesaDrawable);
    if (status != MESA_STATUS_OK) {
        qWarning("mesa.library: MesaMakeCurrent failed (status %d)", (int) status);
        return false;
    }
    return true;
}

void QAmigaOpenGLContext::doneCurrent()
{
    IMesa->MesaUnbindCurrent();
}

QFunctionPointer QAmigaOpenGLContext::getProcAddress(const char *procName)
{
    return QFunctionPointer(IMesa->MesaGetProcAddress(procName));
}

QT_END_NAMESPACE
