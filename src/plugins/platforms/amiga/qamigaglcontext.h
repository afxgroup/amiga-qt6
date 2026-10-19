#ifndef QAMIGAOPENGLCONTEXT_p_h
#define QAMIGAOPENGLCONTEXT_p_h

#include <QtCore/qhash.h>
#include <qpa/qplatformopenglcontext.h>
#include <qpa/qplatformsurface.h>

#include "qamigawindow.h"
#include "qamigaoffscreensurface.h"

#include <proto/exec.h>
#include <proto/mesa.h>
#include <mesa/mesa_defs.h>
#include <proto/graphics.h>

QT_BEGIN_NAMESPACE

extern struct Library *MesaBase;
extern struct MesaIFace *IMesa;

// Desktop OpenGL through mesa.library. The lifecycle follows GLFW's
// AmigaOS4 backend: an Intuition window gets a MesaDrawable, the context is
// created on a drawable, and the drawable is recreated when the surface moves
// to another window.
class QAmigaOpenGLContext : public QPlatformOpenGLContext
{
private:
    static int noContexts;

private:
    MesaContext mesaContext;
    MesaDrawable mesaDrawable;
    struct Window *drawableWindow;
    MesaContext share;
    QSurfaceFormat requestedFormat;
    QOpenGLContext *context;
    QPlatformSurface *surface;
    struct Window *dummyW;

    bool createContext();
    bool createDrawable(struct Window *window);
    void destroyDrawable();

public:
    QAmigaOpenGLContext(QOpenGLContext *context);
    ~QAmigaOpenGLContext();

    QSurfaceFormat format() const override;

    void swapBuffers(QPlatformSurface *platformSurface) override;

    bool isSharing() const override
    {
        return context->shareContext();
    }

    bool makeCurrent(QPlatformSurface *platformSurface) override;

    void doneCurrent() override;

    QFunctionPointer getProcAddress(const char *procName) override;
};

QT_END_NAMESPACE

#endif
