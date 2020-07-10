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
    initEditMenu();
    initEditFragmentsMenu();
    initFileMenu();
    initToolBars();

    this->setWindowTitle("MedMax");
}

MainWindow::~MainWindow()
{

}

void MainWindow::initDisplayDockWidgets(){

    skullDockWidget = new QDockWidget("Plane controls");
    skullDockWidget->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
    skullDockWidget->setVisible(false);

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
    contentLayoutMand->addRow("Green", leftPlaneSlider);

    QSlider *rightPlaneSlider = new QSlider(Qt::Horizontal);
    rightPlaneSlider->setMaximum(sliderMax);
    contentLayoutMand->addRow("Red", rightPlaneSlider);

    connect(leftPlaneSlider, static_cast<void (QSlider::*)(int)>(&QSlider::sliderMoved), skullViewer, &Viewer::moveLeftPlane);
    connect(rightPlaneSlider, static_cast<void (QSlider::*)(int)>(&QSlider::sliderMoved), skullViewer, &Viewer::moveRightPlane);

    // Connect the two views
    connect(skullViewer, &Viewer::polylineBent, fibulaViewer, &ViewerFibula::bendPolylineNormals);

    QSlider *rotatePolylineMandible = new QSlider(Qt::Horizontal);
   rotatePolylineMandible->setMaximum(360);
   contentLayoutMand->addRow("Rotate planes", rotatePolylineMandible);

    connect(rotatePolylineMandible, static_cast<void (QSlider::*)(int)>(&QSlider::sliderMoved), skullViewer, &Viewer::rotatePolylineOnAxis);
    connect(skullViewer, &Viewer::toRotatePolylineOnAxis, fibulaViewer, &ViewerFibula::rotatePolylineOnAxisFibula);

    QSlider *rotatePoly = new QSlider(Qt::Horizontal);
    rotatePolylineMandible->setMaximum(360);
    contentLayoutMand->addRow("Rotate polyline", rotatePoly);

    connect(rotatePoly, static_cast<void (QSlider::*)(int)>(&QSlider::sliderMoved), fibulaViewer, &ViewerFibula::rotatePolylineOnAxe);

    layout->addWidget(contentsMand);
    layout->addWidget(contentsFibula);

    QWidget* controlWidget = new QWidget();
    controlWidget->setLayout(layout);

    skullDockWidget->setWidget(controlWidget);

    this->addDockWidget(Qt::BottomDockWidgetArea, skullDockWidget);
}

void MainWindow::initEditMenu(){
    editMenuWidget = new QDockWidget("Edit Menu");
    editMenuWidget->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);

    QVBoxLayout* layout = new QVBoxLayout();

    QPushButton *toggleDrawPlanesButton = new QPushButton(tr("&Draw planes"));
    toggleDrawPlanesButton->setCheckable(true);
    toggleDrawPlanesButton->setChecked(true);

    connect(toggleDrawPlanesButton, &QPushButton::released, skullViewer, &Viewer::toggleDrawPlane);
    connect(toggleDrawPlanesButton, &QPushButton::released, fibulaViewer, &ViewerFibula::toggleDrawPlane);

    QPushButton *toggleDrawCurveButton = new QPushButton(tr("&Draw curve"));
    toggleDrawCurveButton->setCheckable(true);
    toggleDrawCurveButton->setChecked(false);

    connect(toggleDrawCurveButton, &QPushButton::released, skullViewer, &Viewer::toggleDrawCurve);
    connect(toggleDrawCurveButton, &QPushButton::released, fibulaViewer, &ViewerFibula::toggleDrawCurve);

    // Adjust the plane transparency
    QGroupBox *sliderBox = new QGroupBox("Transparencies", this);
    QHBoxLayout *sliderBoxLayout = new QHBoxLayout();

    QSlider *planeAlphaSlider = new QSlider(Qt::Vertical);
    planeAlphaSlider->setMaximum(100);
    planeAlphaSlider->setSliderPosition(50);
    connect(planeAlphaSlider, static_cast<void (QSlider::*)(int)>(&QSlider::sliderMoved), skullViewer, &Viewer::setPlaneAlpha);
    connect(planeAlphaSlider, static_cast<void (QSlider::*)(int)>(&QSlider::sliderMoved), fibulaViewer, &ViewerFibula::setPlaneAlpha);

    QGroupBox *planeBox = new QGroupBox();
    QVBoxLayout *planeBoxLayout = new QVBoxLayout();
    QLabel *planeLabel = new QLabel();
    planeLabel->setText("Planes");
    planeBoxLayout->addWidget(planeLabel);
    planeBoxLayout->addWidget(planeAlphaSlider);
    planeBox->setLayout(planeBoxLayout);

    QSlider *meshAlphaSlider = new QSlider(Qt::Vertical);
    meshAlphaSlider->setMaximum(100);
    meshAlphaSlider->setSliderPosition(100);
    connect(meshAlphaSlider, static_cast<void (QSlider::*)(int)>(&QSlider::sliderMoved), skullViewer, &Viewer::setMeshAlpha);
    connect(meshAlphaSlider, static_cast<void (QSlider::*)(int)>(&QSlider::sliderMoved), fibulaViewer, &ViewerFibula::setMeshAlpha);

    QGroupBox *meshBox = new QGroupBox();
    QVBoxLayout *meshBoxLayout = new QVBoxLayout();
    QLabel *meshLabel = new QLabel();
    meshLabel->setText("Mesh");
    meshBoxLayout->addWidget(meshLabel);
    meshBoxLayout->addWidget(meshAlphaSlider);
    meshBox->setLayout(meshBoxLayout);

    sliderBoxLayout->addWidget(planeBox);
    sliderBoxLayout->addWidget(meshBox);
    sliderBox->setLayout(sliderBoxLayout);

    layout->addWidget(toggleDrawPlanesButton);
    layout->addWidget(toggleDrawCurveButton);
    layout->addWidget(sliderBox);

    QWidget* controlWidget = new QWidget();
    controlWidget->setLayout(layout);

    editMenuWidget->setWidget(controlWidget);
    this->addDockWidget(Qt::RightDockWidgetArea, editMenuWidget);

    connect(skullViewer, &Viewer::enableFragmentEditing, this, &MainWindow::enableFragmentEditing);
    connect(skullViewer, &Viewer::disableFragmentEditing, this, &MainWindow::disableFragmentEditing);

    editMenuWidget->setVisible(false);
}

void MainWindow::initEditFragmentsMenu(){
    editFragmentDockWidget = new QDockWidget("Edit Fragments");
    editFragmentDockWidget->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);

    QVBoxLayout* layout = new QVBoxLayout();

    groupRadioBox = new QGroupBox(tr("Edit fragments"));

    radioFrag1 = new QRadioButton("Centre", this);
    radioFrag2 = new QRadioButton("Left", this);
    radioFrag3 = new QRadioButton("Right", this);
    radioFragPlanes = new QRadioButton("Planes", this);

    connect(radioFrag1, &QRadioButton::toggled, skullViewer, &Viewer::toggleEditBoxMode);
    connect(radioFrag2, &QRadioButton::toggled, skullViewer, &Viewer::toggleEditFirstCorner);
    connect(radioFrag3, &QRadioButton::toggled, skullViewer, &Viewer::toggleEditEndCorner);
    connect(radioFragPlanes, &QPushButton::toggled, skullViewer, &Viewer::toggleEditPlaneMode);

    QVBoxLayout *vbox = new QVBoxLayout;

    vbox->addWidget(radioFragPlanes);
    vbox->addWidget(radioFrag1);
    vbox->addWidget(radioFrag2);
    vbox->addWidget(radioFrag3);
    vbox->addStretch(1);
    groupRadioBox->setLayout(vbox);

    connect(groupRadioBox, &QGroupBox::clicked, this, &MainWindow::setFragRadios);

    toggleDrawMeshButton = new QPushButton(tr("&Draw mesh"));
    toggleDrawMeshButton->setCheckable(true);
    toggleDrawMeshButton->setChecked(false);

    connect(toggleDrawMeshButton, &QPushButton::released, skullViewer, &Viewer::toggleDrawMesh);

    QPushButton *drawPolylineButton = new QPushButton("Draw Polyline", this);
    connect(drawPolylineButton, &QPushButton::clicked, skullViewer, &Viewer::toggleDrawPolyline);
    connect(drawPolylineButton, &QPushButton::clicked, fibulaViewer, &ViewerFibula::toggleDrawPolyline);

    QPushButton *drawBoxesButton = new QPushButton("Draw Boxes", this);
    connect(drawBoxesButton, &QPushButton::clicked, skullViewer, &Viewer::toggleDrawBoxes);
    connect(drawBoxesButton, &QPushButton::clicked, fibulaViewer, &ViewerFibula::toggleDrawBoxes);

    layout->addWidget(toggleDrawMeshButton);
    layout->addWidget(drawPolylineButton);
    layout->addWidget(drawBoxesButton);
    layout->addWidget(groupRadioBox);

    QWidget *controlWidget = new QWidget();
    controlWidget->setLayout(layout);

    editFragmentDockWidget->setWidget(controlWidget);
    this->addDockWidget(Qt::RightDockWidgetArea, editFragmentDockWidget);

    disableFragmentEditing();
}

void MainWindow::displayEditMenu(){
    if(editMenuWidget->isVisible()) editMenuWidget->setVisible(false);
    else editMenuWidget->setVisible(true);
}

void MainWindow::displayEditFragmentMenu(){
    if(editFragmentDockWidget->isVisible()) editFragmentDockWidget->setVisible(false);
    else editFragmentDockWidget->setVisible(true);
}

void MainWindow::enableFragmentEditing(){
    groupRadioBox->setCheckable(true);
    groupRadioBox->setChecked(false);

    radioFrag1->setCheckable(true);
    radioFrag2->setCheckable(true);
    radioFrag3->setCheckable(true);
    radioFragPlanes->setCheckable(true);
    editFragmentDockWidget->setVisible(true);
}

void MainWindow::disableFragmentEditing(){
    groupRadioBox->setCheckable(false);

    radioFrag1->setCheckable(false);
    radioFrag2->setCheckable(false);
    radioFrag3->setCheckable(false);
    radioFragPlanes->setCheckable(false);

    editFragmentDockWidget->setVisible(false);
}

void MainWindow::setFragRadios(){
    if(!groupRadioBox->isChecked()){
        radioFrag1->setAutoExclusive(false);
        radioFrag1->setChecked(false);
        radioFrag2->setAutoExclusive(false);
        radioFrag2->setChecked(false);
        radioFrag3->setAutoExclusive(false);
        radioFrag3->setChecked(false);
        radioFragPlanes->setAutoExclusive(false);
        radioFragPlanes->setChecked(false);
    }
    else{
        radioFrag1->setAutoExclusive(true);
        radioFrag2->setAutoExclusive(true);
        radioFrag3->setAutoExclusive(true);
        radioFragPlanes->setAutoExclusive(true);
        radioFragPlanes->setChecked(true);
    }
}

void MainWindow::displayFragmentMenuButton(){
    /*editFragmentMenuButton->setVisible(true);
    editFragmentMenuButton->setChecked(true);*/
   //skullDockWidget->setVisible(false);
}

void MainWindow::hideFragmentMenuButton(){
    //editFragmentMenuButton->setVisible(false);
    //skullDockWidget->setVisible(true);
}

void MainWindow::initFileActions(){
    fileActionGroup = new QActionGroup(this);

    QAction *openJsonFileAction = new QAction("Open Mandible", this);
    connect(openJsonFileAction, &QAction::triggered, this, &MainWindow::openMandJSON);

    QAction *openJsonFibFileAction = new QAction("Open Fibula", this);
    connect(openJsonFibFileAction, &QAction::triggered, this, &MainWindow::openFibJSON);

    fileActionGroup->addAction(openJsonFileAction);
    fileActionGroup->addAction(openJsonFibFileAction);

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

    loadedMeshes = new QWidget();
    QHBoxLayout *loadedMeshesLayout = new QHBoxLayout();

    QPushButton *cutMeshAction = new QPushButton("Cut", this);
    connect(cutMeshAction, &QPushButton::clicked, skullViewer, &Viewer::cutMesh);
    connect(cutMeshAction, &QPushButton::clicked, this, &MainWindow::displayFragmentMenuButton);

    QPushButton *uncutMeshAction = new QPushButton("Undo Cut", this);
    connect(uncutMeshAction, &QPushButton::clicked, skullViewer, &Viewer::uncutMesh);
    connect(uncutMeshAction, &QPushButton::clicked, fibulaViewer, &Viewer::uncutMesh);
    connect(uncutMeshAction, &QPushButton::clicked, this, &MainWindow::hideFragmentMenuButton);

    editMenuButton = new QPushButton("Edit Graphics", this);
    connect(editMenuButton, &QPushButton::clicked, this, &MainWindow::displayEditMenu);
    editMenuButton->setCheckable(true);
    editMenuButton->setChecked(false);

    editFragmentMenuButton = new QPushButton("Edit Fragments", this);
    connect(editFragmentMenuButton, &QPushButton::clicked, this, &MainWindow::displayEditFragmentMenu);
    editFragmentMenuButton->setCheckable(true);
    editFragmentMenuButton->setChecked(false);

    loadedMeshesLayout->addWidget(cutMeshAction);
    loadedMeshesLayout->addWidget(uncutMeshAction);
    loadedMeshesLayout->addWidget(editFragmentMenuButton);
    loadedMeshes->setLayout(loadedMeshesLayout);
    // loadedMeshes->setVisible(false);

    fileToolBar->addWidget(editMenuButton);
    fileToolBar->addWidget(loadedMeshes);

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

bool MainWindow::openJSON(Viewer *v){
    QString openFileNameLabel, selectedFilter;

    QString fileFilter = "JSON (*.json)";
    QString filename = QFileDialog::getOpenFileName(this, tr("Select a mesh"), openFileNameLabel, fileFilter, &selectedFilter);

    QFile loadFile(filename);

    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open file.");
        return false;
    }

    QByteArray saveData = loadFile.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));

    readJSON(loadDoc.object(), v);
    return true;
}

void MainWindow::openMandJSON(){
    bool isOpen = openJSON(skullViewer);
    if(!isOpenMand) isOpenMand = isOpen;
    filesOpened();
}

void MainWindow::openFibJSON(){
    bool isOpen = openJSON(fibulaViewer);
    if(!isOpenFib) isOpenFib = isOpen;
    filesOpened();
}

void MainWindow::filesOpened(){
    if(isOpenMand && isOpenFib){
        //loadedMeshes->setVisible(true);
        skullDockWidget->setVisible(true);
    }
}



