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
    initEditMenu();

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

    connect(leftPlaneSlider, static_cast<void (QSlider::*)(int)>(&QSlider::sliderMoved), skullViewer, &Viewer::moveLeftPlane);
    connect(rightPlaneSlider, static_cast<void (QSlider::*)(int)>(&QSlider::sliderMoved), skullViewer, &Viewer::moveRightPlane);

    QSlider *planeAlphaSlider = new QSlider(Qt::Horizontal);
    planeAlphaSlider->setMaximum(100);
    planeAlphaSlider->setSliderPosition(50);
    contentLayoutMand->addRow("Plane transparency", planeAlphaSlider);
    connect(planeAlphaSlider, static_cast<void (QSlider::*)(int)>(&QSlider::sliderMoved), skullViewer, &Viewer::setPlaneAlpha);
    connect(planeAlphaSlider, static_cast<void (QSlider::*)(int)>(&QSlider::sliderMoved), fibulaViewer, &ViewerFibula::setPlaneAlpha);

    // Connect the two views
    connect(skullViewer, &Viewer::polylineBent, fibulaViewer, &ViewerFibula::bendPolylineNormals);

    layout->addWidget(contentsMand);
    layout->addWidget(contentsFibula);

    QWidget* controlWidget = new QWidget();
    controlWidget->setLayout(layout);

    skullDockWidget->setWidget(controlWidget);

    this->addDockWidget(Qt::BottomDockWidgetArea, skullDockWidget);
}

void MainWindow::initEditMenu(){
    editMenuWidget = new QDockWidget("Edit Menu");

    QVBoxLayout* layout = new QVBoxLayout();

    groupRadioBox = new QGroupBox(tr("Edit fragments"));

    radioFrag1 = new QRadioButton(tr("&Radio button 1"));
    radioFrag2 = new QRadioButton(tr("R&adio button 2"));
    radioFrag3 = new QRadioButton(tr("Ra&dio button 3"));

    connect(skullViewer, &Viewer::enableFragmentEditing, this, &MainWindow::enableFragmentEditing);
    connect(skullViewer, &Viewer::disableFragmentEditing, this, &MainWindow::disableFragmentEditing);

    disableFragmentEditing();

    connect(radioFrag1, &QRadioButton::toggled, skullViewer, &Viewer::toggleEditBoxMode);
    connect(radioFrag2, &QRadioButton::toggled, skullViewer, &Viewer::toggleEditFirstCorner);
    connect(radioFrag3, &QRadioButton::toggled, skullViewer, &Viewer::toggleEditEndCorner);

    QVBoxLayout *vbox = new QVBoxLayout;

    vbox->addWidget(radioFrag1);
    vbox->addWidget(radioFrag2);
    vbox->addWidget(radioFrag3);
    vbox->addStretch(1);
    groupRadioBox->setLayout(vbox);

    connect(groupRadioBox, &QGroupBox::clicked, this, &MainWindow::setFragRadios);

    QPushButton *toggleDrawMeshButton = new QPushButton(tr("&Draw mesh"));
    toggleDrawMeshButton->setCheckable(true);
    toggleDrawMeshButton->setChecked(false);

    connect(toggleDrawMeshButton, &QPushButton::released, skullViewer, &Viewer::toggleDrawMesh);

    QPushButton *toggleDrawPlanesButton = new QPushButton(tr("&Draw planes"));
    toggleDrawPlanesButton->setCheckable(true);
    toggleDrawPlanesButton->setChecked(true);

    connect(toggleDrawPlanesButton, &QPushButton::released, skullViewer, &Viewer::toggleDrawPlane);
    connect(toggleDrawPlanesButton, &QPushButton::released, fibulaViewer, &ViewerFibula::toggleDrawPlane);

    layout->addWidget(toggleDrawPlanesButton);
    layout->addWidget(toggleDrawMeshButton);
    layout->addWidget(groupRadioBox);

    QWidget* controlWidget = new QWidget();
    controlWidget->setLayout(layout);

    editMenuWidget->setWidget(controlWidget);

    this->addDockWidget(Qt::RightDockWidgetArea, editMenuWidget);
}

void MainWindow::enableFragmentEditing(){
    groupRadioBox->setCheckable(true);
    groupRadioBox->setChecked(false);

    radioFrag1->setCheckable(true);
    radioFrag2->setCheckable(true);
    radioFrag3->setCheckable(true);
}

void MainWindow::disableFragmentEditing(){
    groupRadioBox->setCheckable(false);

    radioFrag1->setCheckable(false);
    radioFrag2->setCheckable(false);
    radioFrag3->setCheckable(false);
}

void MainWindow::setFragRadios(){
    if(!groupRadioBox->isChecked()){
        radioFrag1->setAutoExclusive(false);
        radioFrag1->setChecked(false);
        radioFrag2->setAutoExclusive(false);
        radioFrag2->setChecked(false);
        radioFrag3->setAutoExclusive(false);
        radioFrag3->setChecked(false);
    }
    else{
        radioFrag1->setAutoExclusive(true);
        radioFrag1->setChecked(true);
        radioFrag2->setAutoExclusive(true);
        radioFrag3->setAutoExclusive(true);
    }
}

void MainWindow::initFileActions(){
    fileActionGroup = new QActionGroup(this);

    QAction *openJsonFileAction = new QAction("Open mandible JSON", this);
    connect(openJsonFileAction, &QAction::triggered, this, &MainWindow::openMandJSON);

    QAction *openJsonFibFileAction = new QAction("Open fibula JSON", this);
    connect(openJsonFibFileAction, &QAction::triggered, this, &MainWindow::openFibJSON);

    QAction *editPlanesAction = new QAction("Edit menu", this);
    connect(editPlanesAction, &QAction::triggered, this, &MainWindow::initEditMenu);

    QAction *cutMeshAction = new QAction("Cut", this);
    connect(cutMeshAction, &QAction::triggered, skullViewer, &Viewer::cutMesh);

    QAction *uncutMeshAction = new QAction("Undo cut", this);
    connect(uncutMeshAction, &QAction::triggered, skullViewer, &Viewer::uncutMesh);
    connect(uncutMeshAction, &QAction::triggered, fibulaViewer, &Viewer::uncutMesh);

    fileActionGroup->addAction(openJsonFileAction);
    fileActionGroup->addAction(openJsonFibFileAction);
    fileActionGroup->addAction(editPlanesAction);
    fileActionGroup->addAction(cutMeshAction);
    fileActionGroup->addAction(uncutMeshAction);

    connect(skullViewer, &Viewer::constructPoly, fibulaViewer, &ViewerFibula::constructPolyline);
    connect(fibulaViewer, &ViewerFibula::okToPlacePlanes, skullViewer, &Viewer::placePlanes);
    connect(skullViewer, &Viewer::toUpdateDistances, fibulaViewer, &ViewerFibula::updateDistances);
    connect(skullViewer, &Viewer::toUpdatePlaneOrientations, fibulaViewer, &ViewerFibula::updatePlaneOrientations);
    connect(skullViewer, &Viewer::toRotatePolylineOnAxis, fibulaViewer, &ViewerFibula::rotatePolylineOnAxisFibula);
    connect(skullViewer, &Viewer::planeMoved, fibulaViewer, &ViewerFibula::movePlanes);
    connect(fibulaViewer, &ViewerFibula::sendToManible, skullViewer, &Viewer::recieveFromFibulaMesh);
    connect(fibulaViewer, &ViewerFibula::requestNewNorms, skullViewer, &Viewer::sendNewNorms);
    connect(skullViewer, &Viewer::cutFibula, fibulaViewer, &ViewerFibula::cut);
    connect(skullViewer, &Viewer::uncutFibula, fibulaViewer, &ViewerFibula::uncut);
    connect(skullViewer, &Viewer::toReinitBox, fibulaViewer, &ViewerFibula::reinitBox);
    connect(skullViewer, &Viewer::toReinitPoly, fibulaViewer, &ViewerFibula::reinitPoly);
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
        fileName = QDir().relativeFilePath(fileName);
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
        qWarning("Couldn't open file.");
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

// Stage 1
void MainWindow::uncutStage(){
    fileActionGroup = new QActionGroup(this);

    QAction *openJsonFileAction = new QAction("Open mandible JSON", this);
    connect(openJsonFileAction, &QAction::triggered, this, &MainWindow::openMandJSON);

    QAction *openJsonFibFileAction = new QAction("Open fibula JSON", this);
    connect(openJsonFibFileAction, &QAction::triggered, this, &MainWindow::openFibJSON);

    QAction *cutMeshAction = new QAction("Cut", this);
    connect(cutMeshAction, &QAction::triggered, this, &MainWindow::cutStage);

    fileActionGroup->addAction(openJsonFileAction);
    fileActionGroup->addAction(openJsonFibFileAction);
    fileActionGroup->addAction(cutMeshAction);

    // skullViewer->uncutMesh();
}

// Stage 2
void MainWindow::cutStage(){
    fileActionGroup = new QActionGroup(this);

    QAction *editPlanesAction = new QAction("Edit planes", this);
    connect(editPlanesAction, &QAction::triggered, skullViewer, &Viewer::toggleEditPlaneMode);

    QAction *editBoxAction = new QAction("Edit segments", this);
    connect(editBoxAction, &QAction::triggered, skullViewer, &Viewer::toggleEditBoxMode);

    QAction *uncutMeshAction = new QAction("Undo cut", this);
    connect(uncutMeshAction, &QAction::triggered, skullViewer, &Viewer::uncutMesh);
    connect(uncutMeshAction, &QAction::triggered, fibulaViewer, &Viewer::uncutMesh);
;
    fileActionGroup->addAction(editPlanesAction);
    fileActionGroup->addAction(editBoxAction);
    fileActionGroup->addAction(uncutMeshAction);

    skullViewer->cutMesh();
}

// Stage 3
void MainWindow::editStage(){
    fileActionGroup = new QActionGroup(this);

    QAction *drawMeshAction = new QAction("Draw mesh", this);
    connect(drawMeshAction, &QAction::triggered, skullViewer, &Viewer::toggleDrawMesh);
    connect(drawMeshAction, &QAction::triggered, fibulaViewer, &ViewerFibula::toggleDrawMesh);

    QAction *drawPlanesAction = new QAction("Draw planes", this);
    connect(drawPlanesAction, &QAction::triggered, skullViewer, &Viewer::toggleDrawPlane);
    connect(drawPlanesAction, &QAction::triggered, fibulaViewer, &ViewerFibula::toggleDrawPlane);

    QAction *wireframeAction = new QAction("Box wireframe", this);
    connect(wireframeAction, &QAction::triggered, skullViewer, &Viewer::toggleWireframe);
    connect(wireframeAction, &QAction::triggered, fibulaViewer, &ViewerFibula::toggleWireframe);

    QAction *cutMeshAction = new QAction("Cut", this);
    connect(cutMeshAction, &QAction::triggered, this, &MainWindow::cutStage);

    fileActionGroup->addAction(drawMeshAction);
    fileActionGroup->addAction(drawPlanesAction);
    fileActionGroup->addAction(wireframeAction);
}

