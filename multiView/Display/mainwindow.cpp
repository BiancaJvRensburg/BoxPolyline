#include "mainwindow.h"
#include <QLayout>
#include <QGroupBox>
#include <QDockWidget>
#include <QSlider>
#include <QFormLayout>
#include <QPushButton>
#include <QToolBar>
#include <QMenu>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    if (this->objectName().isEmpty())
        this->setObjectName("window");
    this->resize(929, 891);

    // The qglviewer
    //QString skullFilename = "C:\\Users\\Medmax\\Documents\\Project\\Mand_B.off";
    StandardCamera *sc = new StandardCamera();
    skullViewer = new Viewer(this, sc, sliderMax);

    // The fibula viewer
    //QString fibulaFilename = "C:\\Users\\Medmax\\Documents\\Project\\Fibula_G.off";
    StandardCamera *scFibula = new StandardCamera();
    fibulaViewer = new ViewerFibula(this, scFibula, sliderMax, fibulaOffsetMax);

    // Main widget
    QWidget *mainWidget = new QWidget(this);
    //QWidget *fibulaWidget = new QWidget(this);

    // Horizontal layout
    QHBoxLayout *windowLayout = new QHBoxLayout();

    // Add the viewer to the layout
    windowLayout->addWidget(skullViewer);
    windowLayout->addWidget(fibulaViewer);

    // Add the layout to the main widget
    mainWidget->setLayout(windowLayout);

    QGroupBox * viewerGroupBox = new QGroupBox();

    QGridLayout * gridLayout = new QGridLayout(viewerGroupBox);
    gridLayout->setObjectName("gridLayout");

    gridLayout->addWidget(mainWidget, 0, 1, 1, 1);

    viewerGroupBox->setLayout(gridLayout);

    this->setCentralWidget(viewerGroupBox);

    initDisplayDockWidgets();
    initFileMenu();
    initToolBars();

    this->setWindowTitle("MedMax");
}

MainWindow::~MainWindow()
{

}

void MainWindow::initDisplayDockWidgets(){

    skullDockWidget = new QDockWidget("Plane controls");

    QHBoxLayout* layout = new QHBoxLayout();

    // The contents of the dockWidget
    QWidget *contentsMand = new QWidget();
    QFormLayout *contentLayoutMand = new QFormLayout();

    QWidget *contentsFibula = new QWidget();
    QFormLayout *contentLayoutFibula = new QFormLayout();

    contentsMand->setLayout(contentLayoutMand);
    contentsFibula->setLayout(contentLayoutFibula);

    QSlider *leftPlaneSlider = new QSlider(Qt::Horizontal);
    leftPlaneSlider->setMaximum(sliderMax);
    contentLayoutMand->addRow("Left slider", leftPlaneSlider);

    QSlider *rightPlaneSlider = new QSlider(Qt::Horizontal);
    rightPlaneSlider->setMaximum(sliderMax);
    contentLayoutMand->addRow("Right slider", rightPlaneSlider);

    QSlider *rotatePolylineMandible = new QSlider(Qt::Horizontal);
    rotatePolylineMandible->setMaximum(360);
    contentLayoutMand->addRow("Rotate planes", rotatePolylineMandible);

    /*QSlider *rotatePolylineFibula = new QSlider(Qt::Horizontal);
    rotatePolylineFibula->setMaximum(360);
    contentLayoutMand->addRow("Rotate planes (fibula)", rotatePolylineFibula);*/

    connect(leftPlaneSlider, static_cast<void (QSlider::*)(int)>(&QSlider::sliderMoved), skullViewer, &Viewer::moveLeftPlane);
    connect(rightPlaneSlider, static_cast<void (QSlider::*)(int)>(&QSlider::sliderMoved), skullViewer, &Viewer::moveRightPlane);
    //connect(rotatePolylineFibula, static_cast<void (QSlider::*)(int)>(&QSlider::sliderMoved), fibulaViewer, &ViewerFibula::rotatePolylineOnAxis);
    connect(rotatePolylineMandible, static_cast<void (QSlider::*)(int)>(&QSlider::sliderMoved), skullViewer, &Viewer::rotatePolylineOnAxis);

    QSlider *meshAlphaSlider = new QSlider(Qt::Horizontal);
    meshAlphaSlider->setMaximum(100);
    meshAlphaSlider->setSliderPosition(100);
    contentLayoutMand->addRow("Mesh transparency", meshAlphaSlider);
    connect(meshAlphaSlider, static_cast<void (QSlider::*)(int)>(&QSlider::sliderMoved), skullViewer, &Viewer::setMeshAlpha);
    connect(meshAlphaSlider, static_cast<void (QSlider::*)(int)>(&QSlider::sliderMoved), fibulaViewer, &ViewerFibula::setMeshAlpha);


    QSlider *planeAlphaSlider = new QSlider(Qt::Horizontal);
    planeAlphaSlider->setMaximum(100);
    planeAlphaSlider->setSliderPosition(50);
    contentLayoutMand->addRow("Plane transparency", planeAlphaSlider);
    connect(planeAlphaSlider, static_cast<void (QSlider::*)(int)>(&QSlider::sliderMoved), skullViewer, &Viewer::setPlaneAlpha);
    connect(planeAlphaSlider, static_cast<void (QSlider::*)(int)>(&QSlider::sliderMoved), fibulaViewer, &ViewerFibula::setPlaneAlpha);

    QSlider *boxAlphaSlider = new QSlider(Qt::Horizontal);
    boxAlphaSlider->setMaximum(100);
    boxAlphaSlider->setSliderPosition(100);
    contentLayoutMand->addRow("Box transparency", boxAlphaSlider);
    connect(boxAlphaSlider, static_cast<void (QSlider::*)(int)>(&QSlider::sliderMoved), skullViewer, &Viewer::setBoxAlpha);
    connect(boxAlphaSlider, static_cast<void (QSlider::*)(int)>(&QSlider::sliderMoved), fibulaViewer, &ViewerFibula::setBoxAlpha);


    // Connect the two views
    connect(skullViewer, &Viewer::polylineBent, fibulaViewer, &ViewerFibula::bendPolylineNormals);

    layout->addWidget(contentsMand);
    layout->addWidget(contentsFibula);

    QWidget* controlWidget = new QWidget();
    controlWidget->setLayout(layout);

    skullDockWidget->setWidget(controlWidget);

    this->addDockWidget(Qt::BottomDockWidgetArea, skullDockWidget);
}

void MainWindow::initFileActions(){
    fileActionGroup = new QActionGroup(this);

    QAction *openJsonFileAction = new QAction("Open mandible JSON", this);
    connect(openJsonFileAction, &QAction::triggered, this, &MainWindow::openMandJSON);

    QAction *openJsonFibFileAction = new QAction("Open fibula JSON", this);
    connect(openJsonFibFileAction, &QAction::triggered, this, &MainWindow::openFibJSON);

    QAction *cutMeshAction = new QAction("Cut", this);
    connect(cutMeshAction, &QAction::triggered, skullViewer, &Viewer::cutMesh);

    QAction *cutAction = new QAction("Cut mesh", this);
    connect(cutAction, &QAction::triggered, skullViewer, &Viewer::cut);
    connect(cutAction, &QAction::triggered, fibulaViewer, &ViewerFibula::cut);

    QAction *uncutAction = new QAction("Uncut mesh", this);
    connect(uncutAction, &QAction::triggered, skullViewer, &Viewer::uncut);
    connect(uncutAction, &QAction::triggered, fibulaViewer, &ViewerFibula::uncut);

    QAction *uncutMeshAction = new QAction("Undo cut", this);
    connect(uncutMeshAction, &QAction::triggered, skullViewer, &Viewer::uncutMesh);
    connect(uncutMeshAction, &QAction::triggered, fibulaViewer, &Viewer::uncutMesh);

    QAction *drawMeshAction = new QAction("Draw mesh", this);
    connect(drawMeshAction, &QAction::triggered, skullViewer, &Viewer::toggleDrawMesh);
    connect(drawMeshAction, &QAction::triggered, fibulaViewer, &ViewerFibula::toggleDrawMesh);

    QAction *drawPlanesAction = new QAction("Draw planes", this);
    connect(drawPlanesAction, &QAction::triggered, skullViewer, &Viewer::toggleDrawPlane);
    connect(drawPlanesAction, &QAction::triggered, fibulaViewer, &ViewerFibula::toggleDrawPlane);

    QAction *wireframeAction = new QAction("Box wireframe", this);
    connect(wireframeAction, &QAction::triggered, skullViewer, &Viewer::toggleWireframe);
    connect(wireframeAction, &QAction::triggered, fibulaViewer, &ViewerFibula::toggleWireframe);

   /* QAction *projectionAction = new QAction("Projection", this);
    connect(projectionAction, &QAction::triggered, fibulaViewer, &ViewerFibula::projectToMesh);*/


    fileActionGroup->addAction(openJsonFileAction);
    fileActionGroup->addAction(openJsonFibFileAction);
    fileActionGroup->addAction(cutMeshAction);
    //fileActionGroup->addAction(projectionAction);
    fileActionGroup->addAction(cutAction);
    fileActionGroup->addAction(uncutAction);
    fileActionGroup->addAction(uncutMeshAction);
    fileActionGroup->addAction(drawMeshAction);
    fileActionGroup->addAction(drawPlanesAction);
    fileActionGroup->addAction(wireframeAction);

    connect(skullViewer, &Viewer::constructPoly, fibulaViewer, &ViewerFibula::constructPolyline);
    connect(fibulaViewer, &ViewerFibula::okToPlacePlanes, skullViewer, &Viewer::placePlanes);
    connect(skullViewer, &Viewer::toUpdateDistances, fibulaViewer, &ViewerFibula::updateDistances);
    connect(skullViewer, &Viewer::toUpdatePlaneOrientations, fibulaViewer, &ViewerFibula::updatePlaneOrientations);
    connect(skullViewer, &Viewer::toRotatePolylineOnAxis, fibulaViewer, &ViewerFibula::rotatePolylineOnAxisFibula);
    connect(skullViewer, &Viewer::planeMoved, fibulaViewer, &ViewerFibula::movePlanes);
    connect(fibulaViewer, &ViewerFibula::sendToManible, skullViewer, &Viewer::recieveFromFibulaMesh);
}

void MainWindow::initFileMenu(){
    initFileActions();

    /*QMenu *fileMenu = menuBar()->addMenu(tr("File"));
    fileMenu->addActions(fileActionGroup->actions());*/
}

void MainWindow::initToolBars () {
    QToolBar *fileToolBar = new QToolBar(this);
    fileToolBar->addActions(fileActionGroup->actions());
    addToolBar(fileToolBar);
}

void MainWindow::readJSON(const QJsonObject &json, Viewer *v){
    if(json.contains("mesh file") && json["mesh file"].isString()){
        QString fileName = json["mesh file"].toString();
        v->openOFF(fileName);
    }
    if(json.contains("control points") && json["control points"].isArray()){
        QJsonArray controlPoints = json["control points"].toArray();
        v->readJSON(controlPoints);
    }
}

void MainWindow::openJSON(Viewer *v){
    QString openFileNameLabel, selectedFilter;

    QString fileFilter = "JSON (*.json)";
    QString filename = QFileDialog::getOpenFileName(this, tr("Select a mesh"), openFileNameLabel, fileFilter, &selectedFilter);


    QFile loadFile(filename);

    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open save file.");
        return;
    }

    QByteArray saveData = loadFile.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));

    readJSON(loadDoc.object(), v);
}

void MainWindow::openMandJSON(){
    openJSON(skullViewer);
}

void MainWindow::openFibJSON(){
    openJSON(fibulaViewer);
}

