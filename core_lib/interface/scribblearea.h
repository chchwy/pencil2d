/*

Pencil - Traditional Animation Software
Copyright (C) 2005-2007 Patrick Corrieri & Pascal Naidon
Copyright (C) 2013-2014 Matt Chang

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation;

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

*/
#ifndef SCRIBBLEAREA_H
#define SCRIBBLEAREA_H

#include <cstdint>
#include <QColor>
#include <QImage>
#include <QPoint>
#include <QWidget>
#include <QFrame>
#include <QHash>
#include "vectorimage.h"
#include "bitmapimage.h"
#include "colourref.h"
#include "vectorselection.h"
#include "basetool.h"
#include "colormanager.h"

class Editor;
class Layer;
class StrokeManager;
class BaseTool;
class ColorManager;
class PopupColorPaletteWidget;

enum DisplayEffect : uint32_t
{
    EFFECT_ANTIALIAS = 0,
    EFFECT_SHADOW,
    EFFECT_PREV_ONION,
    EFFECT_NEXT_ONION,
    EFFECT_AXIS,
    EFFECT_COUNT,
};


class ScribbleArea : public QWidget
{
    Q_OBJECT

    friend class MoveTool;
    friend class EditTool;
    friend class SmudgeTool;

public:
    ScribbleArea( QWidget *parent );
    ~ScribbleArea();

    void setCore( Editor* pCore ) { m_pEditor = pCore; }

    void deleteSelection();
    void setSelection( QRectF rect, bool );
    void displaySelectionProperties();
    QRectF getSelection() const { return mySelection; }
    bool somethingSelected;
    QRectF mySelection, myTransformedSelection, myTempTransformedSelection;
    qreal myRotatedAngle;
    qreal myFlipX, myFlipY; // scale -1.0 or +1.0

    bool areLayersSane() const;
    bool isLayerPaintable() const;

    static QBrush getBackgroundBrush( QString );

    bool isEffectOn( DisplayEffect e ) { return m_effects[ e ]; }

    bool showThinLines() const { return m_showThinLines; }
    int showAllLayers() const { return m_showAllLayers; }
    qreal getCurveSmoothing() const { return curveSmoothing; }
    bool usePressure() const { return m_usePressure; }
    bool makeInvisible() const { return m_makeInvisible; }

    enum MoveMode { MIDDLE, TOPLEFT, TOPRIGHT, BOTTOMLEFT, BOTTOMRIGHT, ROTATION, SYMMETRY };
    MoveMode getMoveMode() const { return m_moveMode; }
    void setMoveMode( MoveMode moveMode ) { m_moveMode = moveMode; }

    QMatrix getView();
    QRectF getViewRect();
    QPointF getCentralPoint();

    qreal getViewScaleX() const { return myView.m11(); }
    qreal getTempViewScaleX() const { return myTempView.m11(); }
    qreal getViewScaleY() const { return myView.m22(); }
    qreal getTempViewScaleY() const { return myTempView.m22(); }
    qreal getCentralViewScale() const { return ( sqrt( centralView.determinant() ) ); }

    QMatrix getTransformationMatrix() const { return transMatrix; }
    void setTransformationMatrix( QMatrix matrix );
    void applyTransformationMatrix();

    void updateCurrentFrame();
    void updateFrame( int frame );
    void updateAllFrames();
    void updateAllVectorLayersAtCurrentFrame();
    void updateAllVectorLayersAt( int frame );
    void updateAllVectorLayers();

    bool shouldUpdateAll() const { return updateAll; }
    void setAllDirty() { updateAll = true; }

    BaseTool* currentTool();
    BaseTool* getTool( ToolType eToolMode );
    void setCurrentTool( ToolType eToolMode );
    void setTemporaryTool( ToolType eToolMode );
    void setPrevTool();

    QPointF pixelToPoint( QPointF pixel );

    StrokeManager *getStrokeManager() const { return m_strokeManager; }

    PopupColorPaletteWidget *getPopupPalette() const { return m_popupPaletteWidget; }

    void keyPressed( QKeyEvent *event );

    Editor* editor() { return m_pEditor; }

signals:
    void modification();
    void modification( int );
    void thinLinesChanged( bool );
    void outlinesChanged( bool );

    void onionPrevChanged( bool );
    void onionNextChanged( bool );
    void multiLayerOnionSkinChanged( bool );

public slots:
    void clearImage();
    void calculateSelectionRect();
    void calculateSelectionTransformation();
    void paintTransformedSelection();
    void setModified( int layerNumber, int frameNumber );

    void selectAll();
    void deselectAll();

    void toggleOnionPrev( bool );
    void toggleOnionNext( bool );
    void toggleOnionBlue( bool );
    void toggleOnionRed( bool );
    void toggleGridA( bool );
    void toggleGridB( bool );
    void grid();

    void resetView();
    void setMyView( QMatrix view );
    QMatrix getMyView();

    void zoom();
    void zoom1();
    void rotatecw();
    void rotateacw();

    void setCurveSmoothing( int );
    void setBackground( int );
    void setBackgroundBrush( QString );
    void toggleThinLines();
    void toggleOutlines();
    void toggleMirror();
    void toggleMirrorV();
    void toggleShowAllLayers();
    void escape();

    void toggleMultiLayerOnionSkin( bool );
    void togglePopupPalette();

    void updateToolCursor();

protected:
    void tabletEvent( QTabletEvent *event ) override;
    void wheelEvent( QWheelEvent *event ) override;
    void mousePressEvent( QMouseEvent *event ) override;
    void mouseMoveEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;
    void mouseDoubleClickEvent( QMouseEvent *event ) override;
    void keyPressEvent( QKeyEvent *event ) override;
    void keyReleaseEvent( QKeyEvent *event ) override;
    void paintEvent( QPaintEvent *event ) override;
    void resizeEvent( QResizeEvent *event ) override;

    void toggledOnionColor();
    void recentre();
    void setView( QMatrix );

public:
    void drawPolyline( QList<QPointF> points, QPointF lastPoint );
    void endPolyline( QList<QPointF> points );

    void drawLine( QPointF P1, QPointF P2, QPen pen, QPainter::CompositionMode cm );
    void drawPath( QPainterPath path, QPen pen, QBrush brush, QPainter::CompositionMode cm );
    void drawBrush( QPointF thePoint, qreal brushWidth, qreal offset, QColor fillColour, qreal opacity );
    void blurBrush( BitmapImage *bmiSource_, QPointF srcPoint_, QPointF thePoint_, qreal brushWidth_, qreal offset_, qreal opacity_ );
    void liquifyBrush( BitmapImage *bmiSource_, QPointF srcPoint_, QPointF thePoint_, qreal brushWidth_, qreal offset_, qreal opacity_ );
    void floodFill( VectorImage *vectorImage, QPoint point, QRgb targetColour, QRgb replacementColour, int tolerance );

    void paintBitmapBuffer();
    void clearBitmapBuffer();
    void refreshBitmap( QRect rect, int rad );
    void refreshVector( QRect rect, int rad );
    void setGaussianGradient( QGradient &gradient, QColor colour, qreal opacity, qreal offset );

private:
    void updateCanvas( int frame, QRect rect );
    void floodFillError( int errorType );

    void renderShadow( QPainter& );

    MoveMode m_moveMode;
    ToolType prevMode;
    ToolType prevToolType; // previous tool (except temporal)

    StrokeManager* m_strokeManager;

    Editor* m_pEditor;

    PopupColorPaletteWidget* m_popupPaletteWidget; // color palette popup (may be enhanced with tools)

    bool m_isSimplified = false;
    bool m_showThinLines;
    int  m_showAllLayers;
    bool m_usePressure = true;
    bool m_makeInvisible;
    bool toolCursors;
    qreal curveSmoothing;
    bool onionPrev = true;
    bool onionNext = false;
    bool onionBlue, onionRed;
    bool m_isMultiLayerOnionSkin; // future use. If required, just add a checkbox to updated it.
    QColor onionColor;

    bool updateAll;

    bool useGridA;
    bool useGridB;

    QBrush backgroundBrush;
public:
    BitmapImage* m_bufferImg; // used to pre-draw vector modifications

private:
    void initDisplayEffect( std::vector< uint32_t >& );
    std::vector< uint32_t > m_effects;

    bool keyboardInUse;
    bool mouseInUse;
    QPointF lastPixel, currentPixel;
    QPointF lastPoint, currentPoint;

    qreal tol;
    QList<int> closestCurves;
    QList<VertexRef> closestVertices;
    QPointF offset;

    //instant tool (temporal eg. eraser)
    bool instantTool; //whether or not using temporal tool

    VectorSelection vectorSelection;
    QMatrix selectionTransformation;

    QMatrix myView, myTempView, centralView, transMatrix;
    QPixmap canvas;

    // debug
    QRectF debugRect;
};

#endif