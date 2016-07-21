#include <QPushButton>
class QtDemo : public QWidget
{
    Q_OBJECT

public:
    QtDemo(QWidget *parent = 0);
    //~QtDemo();
public slots:
    void mClick();

private:

};


#include "QtDemo.h"
#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>

#include <osgWidget/Browser>
#include <osgWidget/Box>
#include <osg/Notify>
#include <osgQt/QGraphicsViewAdapter>
#include <osgQt/QWidgetImage>
#include <osgQt/QWebViewImage>
#include <osgQt/GraphicsWindowQt>

#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QVBoxLayout>
#include <QMainWindow>
#include <QtEvents>
#include <QLabel>

QPushButton *button;
//
QtDemo::QtDemo(QWidget *parent)
    :   QWidget(parent)
{

    connect(button,SIGNAL(clicked()),this,SLOT(mClick()));
}

void QtDemo::mClick()
{
    std::cout<<"clicked"<<std::endl;
}

class ViewFrameThread :public OpenThreads::Thread
{
public:
    ViewFrameThread(osgViewer::ViewerBase *viewBase,bool doQApplicationExit):m_viewerBase(viewBase),m_doQApplicationExit(doQApplicationExit)
    {
    }
    ~ViewFrameThread()
    {
        cancel();
        while(isRunning())
        {
            OpenThreads::Thread::YieldCurrentThread();
        }
    }
    int cancel()
    {
        m_viewerBase->setDone(true);
        return 0;
    }

    void run()
    {
        int result=m_viewerBase->run();
        if(m_doQApplicationExit) QApplication::exit(result);
    }
private:
    osg::ref_ptr<osgViewer::ViewerBase> m_viewerBase;
    bool m_doQApplicationExit;
};

//设计一个button
class MyPushButton :public QPushButton
{
public:
    MyPushButton(const QString &text) : QPushButton(text)
    {
    }
protected:
    virtual void mousePressEvent(QMouseEvent *event)
    {
        std::cout << " push " << std::endl;
    }
};

osg::Camera *createCameraWithQwidget(int x,int y,int w,int h)
{
    osg::DisplaySettings *ds=osg::DisplaySettings::instance().get();
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits(ds);
    traits->windowDecoration = true;
    traits->x = x;
    traits->y = y;
    traits->width = w;
    traits->height = h;
    traits->doubleBuffer = true;

    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    osg::ref_ptr<osgQt::GraphicsWindowQt> graphic = new osgQt::GraphicsWindowQt(traits.get());
    graphic->getGraphWidget()->setAcceptDrops(false);
    graphic->getGraphWidget()->setMinimumSize(10, 10);
    camera->setGraphicsContext(graphic.get());
    camera->setClearColor( osg::Vec4(0.39, 0.39, 0.39, 1.0) );
    GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
    camera->setDrawBuffer(buffer);
    camera->setReadBuffer(buffer);
    camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    camera->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));
    camera->setProjectionMatrixAsPerspective(
        30.0f, static_cast<double>(traits->width)/static_cast<double>(traits->height), 1.0f, 1000.0f );

    return camera.release();
}

int main(int argc,char **argv)
{

    QApplication app(argc, argv);

    //声明节点
    osg::ref_ptr<osg::Group> root  =new osg::Group;
    osg::ref_ptr<osg::Node> node2=osgDB::readNodeFile("glider.osg");
    osg::ref_ptr<osg::Node> nodeButton=new osg::Node;
    QGraphicsScene *scene;
    QGraphicsView view;
    osg::Camera *camera=new osg::Camera;
    osg::ref_ptr<osgViewer::Viewer> viewer=new osgViewer::Viewer;
    //viewer->setCamera(camera);
    viewer->setCameraManipulator(new osgGA::TrackballManipulator);
    //osgQt::GraphicsWindowQt *gw=dynamic_cast<osgQt::GraphicsWindowQt*>(camera->getGraphicsContext());
    //osgQt::GLWidget *glWidget=gw->getGLWidget();
    QWidget *widget=new QWidget;
    //开始设计
    {
        widget->setWindowTitle("Title");
        widget->setLayout(new QVBoxLayout);

        QString text("中文显示 测试。");
        QLabel *label=new QLabel(text);
        label->setWordWrap(true);
        label->setTextInteractionFlags(Qt::TextEditorInteraction);

        QPalette palette = label->palette();
        palette.setColor(QPalette::Highlight, Qt::darkBlue);
        palette.setColor(QPalette::HighlightedText, Qt::white);
        label->setPalette(palette);

        QScrollArea* scrollArea = new QScrollArea;
        scrollArea->setWidget(label);

        widget->layout()->addWidget(scrollArea);
        button=new QPushButton("zheshi  an niu");
        MyPushButton *slotButton=new MyPushButton("这是 信号草按钮");
        widget->layout()->addWidget(button);
        widget->layout()->addWidget(slotButton);
        widget->setGeometry(0, 0, 800, 600);
        //app.connect(slotButton,SIGNAL(clicked()),this,SLOT(mClick()));
        // 加入一个按钮
    }
    QtDemo qtdemo;
    //开始加入osg窗口渲染
    osg::ref_ptr<osgQt::QWidgetImage> widgetImage=new osgQt::QWidgetImage(widget);
    widgetImage->getQWidget()->setAttribute(Qt::WA_TranslucentBackground);
    widgetImage->getQGraphicsViewAdapter()->setBackgroundColor(QColor(0,0,0,0));
    scene=widgetImage->getQGraphicsViewAdapter()->getQGraphicsScene();
    osg::Geometry *quad=osg::createTexturedQuadGeometry(osg::Vec3(0,0,0),osg::Vec3(1,0,0),osg::Vec3(0,1,0), 1, 1);
    osg::Geode *geode=new osg::Geode;
    geode->addDrawable(quad);
    osg::MatrixTransform *mt=new osg::MatrixTransform;
    osg::Texture2D *texture=new osg::Texture2D(widgetImage);
    texture->setResizeNonPowerOfTwoHint(false);
    texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
    texture->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP_TO_EDGE);
    texture->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP_TO_EDGE);
    mt->getOrCreateStateSet()->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
    osgViewer::InteractiveImageHandler *handler;
    if (true)
    {
        mt->setMatrix(osg::Matrix::rotate(osg::Vec3(0,1,0),osg::Vec3(0,0,1)));
        mt->addChild(geode);
        handler=new osgViewer::InteractiveImageHandler(widgetImage);
    }
    else
    {
        camera->setProjectionResizePolicy(osg::Camera::FIXED);
        camera->setProjectionMatrix(osg::Matrix::ortho2D(0,1,0,1));
        camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
        camera->setViewMatrix(osg::Matrix::identity());
        camera->setClearMask(GL_DEPTH_BUFFER_BIT);
        camera->setRenderOrder(osg::Camera::POST_RENDER);
        camera->addChild(geode);
        camera->setViewport(0,0,1024,768);
        mt->addChild(camera);
        handler=new osgViewer::InteractiveImageHandler(widgetImage,texture,camera);
        mt->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
        mt->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON);
        mt->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        mt->getOrCreateStateSet()->setAttribute(new osg::Program);
    }

    osg::Group *overlay=new osg::Group;
    overlay->addChild(mt);
    root->addChild(overlay);
    quad->setEventCallback(handler);
    quad->setCullCallback(handler);

    //加入牛和浏览器窗口
    root->addChild(node2);
    osg::ArgumentParser arguments(&argc,argv);
    osg::ref_ptr<osgQt::QWebViewImage> image=new osgQt::QWebViewImage;
    image->navigateTo("http://www.baidu.com");
    osgWidget::GeometryHints hints(osg::Vec3(0.0f,0.0f,0.0f),
                                           osg::Vec3(1.0f,0.0f,0.0f),
                                           osg::Vec3(0.0f,0.0f,1.0f),
                                           osg::Vec4(1.0f,1.0f,1.0f,1.0f),
                                           osgWidget::GeometryHints::RESIZE_HEIGHT_TO_MAINTAINCE_ASPECT_RATIO);
    osg::ref_ptr<osgWidget::Browser> browser=new osgWidget::Browser;
    //root->addChild(browser);
    browser->assign(image.get(),hints);

    //节点加入viewer中 运行
    viewer->setSceneData(root.get());
    //app.connect(button,SIGNAL(clicked()),button,SLOT(quit()));
    if (true)
    {
        ViewFrameThread viewThread(viewer.get(),true);
        viewThread.startThread();
        return QApplication::exec();
    }
}
