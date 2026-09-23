#ifndef ANNOTATION_H
#define ANNOTATION_H

#include <GraphicsDefs.h>
#include <Rect.h>
#include <String.h>

enum ToolMode {
    MODE_VIEW = 0,
    MODE_ANNOTATE_TEXT,
    MODE_HIGHLIGHT,
    MODE_DRAW_RECT
};

enum AnnotationType {
    ANNOTATION_TEXT = 0,
    ANNOTATION_HIGHLIGHT,
    ANNOTATION_RECT
};

struct Annotation {
    AnnotationType  type;
    BRect           pageUnitBounds; // Coordinate normalizzate [0, 1] sulla pagina
    BString         text;
    rgb_color       color;
};

#endif // ANNOTATION_H
