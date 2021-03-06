#ifndef VIEW_H
#define VIEW_H

#include <glm/glm.hpp>

#include "Scene.h"
#include "FrameBuffer.h"

class Source;
typedef std::list<Source *> SourceList;

class SessionSource;
class Surface;
class Symbol;
class Mesh;
class Frame;

class View
{
public:

    typedef enum {RENDERING = 0, MIXING=1, GEOMETRY=2, LAYER=3, APPEARANCE=4, TRANSITION=5, INVALID=6 } Mode;

    View(Mode m);
    virtual ~View() {}

    inline Mode mode() const { return mode_; }

    virtual void update (float dt);
    virtual void draw ();

    virtual void zoom (float) {}
    virtual void resize (int) {}
    virtual int  size () { return 0; }
    virtual void recenter();
    virtual void centerSource(Source *) {}

    typedef enum {
        Cursor_Arrow = 0,
        Cursor_TextInput,
        Cursor_ResizeAll,
        Cursor_ResizeNS,
        Cursor_ResizeEW,
        Cursor_ResizeNESW,
        Cursor_ResizeNWSE,
        Cursor_Hand,
        Cursor_NotAllowed
    } CursorType;

    typedef struct Cursor {
        CursorType type;
        std::string info;
        Cursor() { type = Cursor_Arrow; info = "";}
        Cursor(CursorType t, std::string i = "") { type = t; info = i;}
    } Cursor;

    // picking of nodes in a view provided a point coordinates in screen coordinates
    virtual std::pair<Node *, glm::vec2> pick(glm::vec2);

    // select sources provided a start and end selection points in screen coordinates
    virtual void select(glm::vec2, glm::vec2);
    virtual void selectAll();

    // drag the view provided a start and an end point in screen coordinates
    virtual Cursor drag (glm::vec2, glm::vec2);

    // grab a source provided a start and an end point in screen coordinates and the picking point
    virtual void initiate();
    virtual void terminate();
    virtual Cursor grab (Source*, glm::vec2, glm::vec2, std::pair<Node *, glm::vec2>) {
        return Cursor();
    }

//    // test mouse over provided a point in screen coordinates and the picking point
//    virtual Cursor over (Source*, glm::vec2, std::pair<Node *, glm::vec2>) {
//        return Cursor();
//    }

    // accessible scene
    Scene scene;

    // reordering scene when necessary
    static bool need_deep_update_;

protected:

    virtual void restoreSettings();
    virtual void saveSettings();

    std::string current_action_;
    uint64_t current_id_;
    Mode mode_;
};


class MixingView : public View
{
public:
    MixingView();

    void draw () override;
    void update (float dt) override;
    void zoom (float factor) override;
    void resize (int) override;
    int  size () override;
    void centerSource(Source *) override;
    void selectAll() override;

    std::pair<Node *, glm::vec2> pick(glm::vec2) override;
    Cursor grab (Source *s, glm::vec2 from, glm::vec2 to, std::pair<Node *, glm::vec2>) override;
    Cursor drag (glm::vec2, glm::vec2) override;

    void setAlpha (Source *s);
    inline float limboScale() { return limbo_scale_; }

private:
    uint textureMixingQuadratic();
    float limbo_scale_;

    Group *slider_root_;
    class Disk *slider_;
    class Disk *button_white_;
    class Disk *button_black_;
    class Disk *stashCircle_;
    class Mesh *mixingCircle_;

};

class RenderView : public View
{
    FrameBuffer *frame_buffer_;
    Surface *fading_overlay_;

public:
    RenderView ();
    ~RenderView ();

    void draw () override;

    void setResolution (glm::vec3 resolution = glm::vec3(0.f));
    glm::vec3 resolution() const { return frame_buffer_->resolution(); }

    void setFading(float f = 0.f);
    float fading() const;

    inline FrameBuffer *frame () const { return frame_buffer_; }
};

class GeometryView : public View
{
public:
    GeometryView();

    void draw () override;
    void update (float dt) override;
    void zoom (float factor) override;
    void resize (int) override;
    int  size () override;

    std::pair<Node *, glm::vec2> pick(glm::vec2 P) override;
    Cursor grab (Source *s, glm::vec2 from, glm::vec2 to, std::pair<Node *, glm::vec2> pick) override;
//    Cursor over (Source *s, glm::vec2 pos, std::pair<Node *, glm::vec2> pick) override;
    Cursor drag (glm::vec2, glm::vec2) override;
    void terminate() override;

private:
    Node *overlay_position_;
    Node *overlay_position_cross_;
    Node *overlay_rotation_;
    Node *overlay_rotation_fix_;
    Node *overlay_rotation_clock_;
    Node *overlay_rotation_clock_hand_;
    Node *overlay_scaling_;
    Node *overlay_scaling_cross_;
    Node *overlay_scaling_grid_;
    bool show_context_menu_;
};

class LayerView : public View
{
public:
    LayerView();

    void update (float dt) override;
    void zoom (float factor) override;
    void resize (int) override;
    int  size () override;

    Cursor grab (Source *s, glm::vec2 from, glm::vec2 to, std::pair<Node *, glm::vec2> pick) override;
    Cursor drag (glm::vec2, glm::vec2) override;

    float setDepth (Source *, float d = -1.f);

private:
    float aspect_ratio;
};

class TransitionView : public View
{
public:
    TransitionView();

    void draw () override;
    void update (float dt) override;
    void zoom (float factor) override;
    void selectAll() override;

    std::pair<Node *, glm::vec2> pick(glm::vec2 P) override;
    Cursor grab (Source *s, glm::vec2 from, glm::vec2 to, std::pair<Node *, glm::vec2> pick) override;
    Cursor drag (glm::vec2, glm::vec2) override;

    void attach(SessionSource *ts);
    class Session *detach();
    void play(bool open);

private:
    Surface *output_surface_;
    Mesh *mark_100ms_, *mark_1s_;
    Switch *gradient_;
    SessionSource *transition_source_;
};


class AppearanceView : public View
{
public:
    AppearanceView();

    void select(glm::vec2, glm::vec2) override;
    void selectAll() override;

    void draw () override;

    void update (float dt) override;
    void zoom (float factor) override;
    void resize (int) override;
    int  size () override;

    std::pair<Node *, glm::vec2> pick(glm::vec2 P) override;
    Cursor grab (Source *s, glm::vec2 from, glm::vec2 to, std::pair<Node *, glm::vec2> pick) override;
    Cursor drag (glm::vec2, glm::vec2) override;
    void terminate() override;

private:

    Source *edit_source_;
    bool need_edit_update_;
    Source *getEditOrCurrentSource();
    void adjustBackground();

    Surface *surfacepreview;    
    Surface *backgroundchecker_;
    Frame *backgroundframe_;
    Mesh *horizontal_line_;
    Mesh *horizontal_mark_;
    bool show_horizontal_scale_;
    Group *vertical_line_;
    Mesh *vertical_mark_;
    bool show_vertical_scale_;
    Symbol *crop_horizontal_;
    Symbol *crop_vertical_;
    Symbol *overlay_position_;
    Symbol *overlay_position_cross_;
    Symbol *overlay_scaling_;
    Symbol *overlay_scaling_cross_;
    Node *overlay_scaling_grid_;
    Symbol *overlay_rotation_;
    Symbol *overlay_rotation_fix_;
    Node *overlay_rotation_clock_;
    Symbol *overlay_rotation_clock_hand_;
    bool show_context_menu_;
};


#endif // VIEW_H
