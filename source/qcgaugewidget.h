/***************************************************************************
**                                                                        **
**  QcGauge, for instrumentation, and real time data measurement          **
**  visualization widget for Qt.                                          **
**  Copyright (C) 2015 Hadj Tahar Berrima                                 **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU Lesser General Public License as        **
**  published by the Free Software Foundation, either version 3 of the    **
**  License, or (at your option) any later version.                       **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU Lesser General Public License for more details.                   **
**                                                                        **
**  You should have received a copy of the GNU Lesser General Public      **
**  License along with this program.                                      **
**  If not, see http://www.gnu.org/licenses/.                             **
**                                                                        **
****************************************************************************
**           Author:  Hadj Tahar Berrima                                  **
**           Website: http://pytricity.com/                               **
**           Contact: berrima_tahar@yahoo.com                             **
**           Date:    1 dec 2014                                          **
**           Version:  1.0                                                **
****************************************************************************/

#ifndef QCGAUGEWIDGET_H
#define QCGAUGEWIDGET_H

#include <QWidget>
#include <QPen>
#include <QRectF>
#include <QImage>


#if defined(QCGAUGE_COMPILE_LIBRARY)
#  define QCGAUGE_DECL  Q_DECL_EXPORT
#elif defined(QCGAUGE_USE_LIBRARY)
#  define QCGAUGE_DECL Q_DECL_IMPORT
#else
#  define QCGAUGE_DECL
#endif

class QcGaugeWidget;
class QcItem;
class QcBackgroundItem;
class QcDegreesItem;
class QcValuesItem;
class QcArcItem;
class QcColorBand;
class QcNeedleItem;
class QcLabelItem;
class QcGlassItem;
class QcAltitudeMeter;

/**
 * A circular gauge widget for instrumentation, and real time data measurement
 * visualization
 */
class QCGAUGE_DECL QcGaugeWidget : public QWidget
{
    Q_OBJECT
signals:
	void sizeChanged(const QSize& Size);

public:
    explicit QcGaugeWidget(QWidget *parent = 0);    
    virtual ~QcGaugeWidget();

    QcBackgroundItem* addBackground(double position);
    QcDegreesItem* addDegrees(double position);
    QcValuesItem* addValues(double position);
    QcArcItem* addArc(double position);
    QcColorBand* addColorBand(double position);
    QcNeedleItem* addNeedle(double position);
    QcLabelItem* addLabel(double position);
    QcGlassItem* addGlass(double position);
    QcAltitudeMeter* addAltitudeMeter(double position);
    void addItem(QcItem* item, double position);
    bool removeItem(QcItem* item);
    QList <QcItem*> items();
    int diameter() const;
    QImage blurShadowImage(QImage& Source) const;
    QBrush shadowBrush() const;
    QPointF shadowOffset() const;

public:
    virtual int heightForWidth(int w) const;

protected:
    virtual void paintEvent(QPaintEvent*);
    virtual void resizeEvent(QResizeEvent* event);

private:
    void updateBufferImages();
    QImage createBufferImage() const;
    QImage mBackgroundBuffer; ///<Image buffer for the background
	QImage mForegeroundBuffer; ///<Image buffer for the foreground
    QList <QcItem*> mItems;
    bool mSizeChanged;
    int mNeedleItemIndex;
};

/**
 * Base class for all visible items in a circular gauge widget
 */
class QCGAUGE_DECL QcItem
{
public:
    explicit QcItem(QcGaugeWidget* ParentWidget);
    virtual ~QcItem() {}
    virtual void draw(QPainter *) = 0;
    virtual int type();

    void setPosition(double percentage);
    double position() const;
    QRectF widgetRect() const;
    QRectF itemRect() const;
    enum Error{InvalidValueRange,InvalidDegreeRange,InvalidStep};


protected:
    static double getRadius(const QRectF &);
    static double getAngle(const QPointF&, const QRectF &tmpRect);
    static QPointF getPoint(double deg, const QRectF &tmpRect);

    QRectF adjustRect(double percentage) const;
    void update();

    QcGaugeWidget *mGaugeWidget;
    double mPosition;
};


/**
 * Base class for all items that are use to build a scale
 */
class QCGAUGE_DECL QcScaleItem : public QcItem
{
public:
    explicit QcScaleItem(QcGaugeWidget* ParentWidget);
    virtual ~QcScaleItem() {}

    void setDegreeRange(double minDegree,double maxDegree);
    void setMinimumValueAngle(double minDegree);
    void setMaximumValueAngle(double maxDegree);

    void setRange(double minValue,double maxValue);
    void setMinimumValue(double minValue);
    void setMaximumValue(double maxValue);
    inline double minimumValue() const {return mMinValue;}
    inline double maximumValue() const {return mMaxValue;}

protected:

    double getDegFromValue(double) const;

    double mMinValue;
    double mMaxValue;
    double mMinDegree;
    double mMaxDegree;

};

/**
 * An item for building backgrounds, foregrounds and borders of the
 * gauge widget
 */
class QCGAUGE_DECL QcBackgroundItem : public QObject, public QcItem
{
	Q_OBJECT
public:
    explicit QcBackgroundItem(QcGaugeWidget* ParentWidget);
    virtual ~QcBackgroundItem();
    void draw(QPainter*);
    void addColor(double position, const QColor& color);
    void clearColors();
    void setDropShadow(bool DropShadow);
    bool dropShadow() const;
    void setBrush(const QBrush& Brush);
    const QBrush& brush() const;

private slots:
	void onWidgetSizeChanged(const QSize& Size);

private:
    void updateDropShadowImage();

    QPen mPen;
    QList<QPair<double,QColor> > mColors;
    QBrush mBrush;
    QImage mDropShadowImage;
    bool mDropShadow;
};


/**
 * An item that is ovelayed on top of all other items to simulate the visual
 * style of a glas
 */
class QcGlassItem : public QcItem
{
public:
    enum GlassType
    {
    	StronglyCurvedGlass,
    	CurvedGlass1,
    	CurvedGlass2,
    	FlatGlass,
    	FlatGlass2
    };

    explicit QcGlassItem(QcGaugeWidget* ParentWidget);
    void draw(QPainter*);
    void setGlassType(GlassType glassType);

private:
    void createStronglyCurvedGlass(QPainterPath& PainterPath, QBrush& Brush);
    void createCurvedGlass1(QPainterPath& PainterPath, QBrush& Brush);
    void createCurvedGlass2(QPainterPath& PainterPath, QBrush& Brush);
    void createCurvedGlass3(QPainterPath& PainterPath, QBrush& Brush);
    void createFlatGlass(QPainterPath& PainterPath, QBrush& Brush);
    void createFlatGlass2(QPainterPath& PainterPath, QBrush& Brush);

    GlassType mGlassType;
};


/**
 * A lable item for display of text and values.
 * You can assign a label to a needle item to get a realtime dispolay of the
 * needle value
 */
class QCGAUGE_DECL QcLabelItem : public QcItem
{
public:
    explicit QcLabelItem(QcGaugeWidget* ParentWidget);
    virtual void draw(QPainter *);
    void setAngle(double);
    double angle() const;
    void setText(const QString &text, bool repaint = true);
    const QString& text() const;
    void setColor(const QColor& color);
    const QColor& color() const;

private:
    double mAngle;
    QString mText;
    QColor mColor;
};

/**
 * An item for display of a visual arc.
 * This arc is the base for a scale
 */
class QCGAUGE_DECL QcArcItem : public QcScaleItem
{
public:
    explicit QcArcItem(QcGaugeWidget* ParentWidget);
    void draw(QPainter*);
    void setColor(const QColor& color);

private:
    QColor mColor;
};


/**
 * The color band creates an arc of different colors
 */
class QCGAUGE_DECL QcColorBand : public QcScaleItem
{
public:
    explicit QcColorBand(QcGaugeWidget* ParentWidget);
    void draw(QPainter*);
    void setColors(const QList<QPair<QColor,double> >& colors);

private:
   QPainterPath createSubBand(double from,double sweep);

   QList<QPair<QColor,double> > mBandColors;
   double mBandStartValue;
};

/**
 * Use the degree item to visualize the notches of a scale
 */
class QCGAUGE_DECL QcDegreesItem : public QcScaleItem
{
public:
    explicit QcDegreesItem(QcGaugeWidget* ParentWidget);
    void draw(QPainter *painter);
    void setStep(double step);
    void setColor(const QColor& color);
    void setSubDegree(bool );
private:
    double mStep;
    QColor mColor;
    bool mSubDegree;
};


/**
 * The needle item shows the value by pointing on a certain value on the
 * scale of the gauge widget
 */
class QCGAUGE_DECL QcNeedleItem : public QObject, public QcScaleItem
{
	Q_OBJECT
public:
    explicit QcNeedleItem(QcGaugeWidget* ParentWidget);
    void draw(QPainter*);
    double value() const;
    void setColor(const QColor & color);
    const QColor& color() const;
    void setBrush(const QBrush& Brush);
    const QBrush& brush() const;
    void setDropShadow(bool DropShadow);
    bool dropShadow() const;

    void setLabel(QcLabelItem*);
    QcLabelItem * label() const;

    enum NeedleType {
    	DiamonNeedle,
    	TriangleNeedle,
    	FeatherNeedle,
    	AltitudeMeterNeedle,
    	CompassNeedle,
    	CustomNeedle};

    void setNeedle(QcNeedleItem::NeedleType needleType);
    void setCustomNeedle(const QVector<QPointF>& NeedlePolygon);

public slots:
    void setValue(double value);
    void setValueRange(double minValue,double maxValue);
    void setMinimumValue(double minValue);
    void setMaximumValue(double maxValue);

private slots:
	void onWidgetSizeChanged(const QSize& Size);

private:
    QPolygonF createDiamonNeedle(double r) const;
    QPolygonF createTriangleNeedle(double r) const;
    QPolygonF createFeatherNeedle(double r) const;
    QPolygonF createAttitudeNeedle(double r) const;
    QPolygonF createCompassNeedle(double r) const;
    QPolygonF createCustomNeedle(double r) const;

    QPolygonF createNeedlePoly(double r) const;
    void updateDropShadowImage();

    QPolygonF mCustomNeedlePoly;
    double mCurrentValue;
    NeedleType mNeedleType;
    QcLabelItem *mLabel;
    QBrush mBrush;
    QImage mDropShadowImage;
    bool mDropShadow;
};


/**
 * A value item paints the values of a scale.
 */
class QCGAUGE_DECL QcValuesItem : public QcScaleItem
{
public:
    explicit QcValuesItem(QcGaugeWidget* ParentWidget);
    void draw(QPainter*);
    void setStep(double step);
    void setColor(const QColor& color);
    void setFont(const QFont& font);
    const QFont& font() const;

private:
    double mStep;
    QColor mColor;
    QFont mFont;
};


/**
 * This is a special item for painting an altitude meter
 */
class QCGAUGE_DECL QcAltitudeMeter : public QcItem
{
public:
    explicit QcAltitudeMeter(QcGaugeWidget* ParentWidget);

    void draw(QPainter *);
    void setCurrentPitch(double pitch);
    void setCurrentRoll(double roll);

private:
    double mRoll;
    double mPitch;
    double mPitchOffset;

    QPolygonF mHandlePoly;
    QPainterPath mStepsPath;

    QPointF intersection(double r,const QPointF& pitchPoint,const QPointF& pt) const;
    double startAngle(const QRectF& tmpRect) const;

    void drawDegrees(QPainter *);
    void drawDegree(QPainter * painter, const QRectF& tmpRect,double deg);
    void drawUpperEllipse(QPainter *,const QRectF&);
    void drawLowerEllipse(QPainter *,const QRectF&);
    void drawPitchSteps(QPainter *,const QRectF&);
    void drawHandle(QPainter *);
    void drawSteps(QPainter *,double);

};

#endif // QCGAUGEWIDGET_H
