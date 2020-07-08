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

    editPlaneButton = new QPushButton(tr("&Edit planes"));
    editPlaneButton->setCheckable(true);
    editPlaneButton->setChecked(false);

    connect(editPlaneButton, &QPushButton::released, skullViewer, &Viewer::toggleEditPlaneMode);

    toggleDrawMeshButton = new QPushButton(tr("&Draw mesh"));
    toggleDrawMeshButton->setCheckable(true);
    toggleDrawMeshButton->setChecked(false);

    connect(toggleDrawMeshButton, &QPushButton::released, skullViewer, &Viewer::toggleDrawMesh);

    layout->addWidget(toggleDrawMeshButton);
    layout->addWidget(editPlaneButton);
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

    /*groupRadioBox->setVisible(true);
    editPlaneButton->setVisible(true);
    toggleDrawMeshButton->setVisible(true);*/
    editFragmentDockWidget->setVisible(true);
}

void MainWindow::disableFragmentEditing(){
    /*groupRadioBox->setVisible(false);
    editPlaneButton->setVisible(false);
    toggleDrawMeshButton->setVisible(false);*/

    groupRadioBox->setCheckable(false);

    radioFrag1->setCheckable(false);
    radioFrag2->setCheckable(false);
    radioFrag3->setCheckable(false);

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
    }
    else{
        radioFrag1->setAutoExclusive(true);
        radioFrag1->setChecked(true);
        radioFrag2->setAutoExclusive(true);
        radioFrag3->setAutoExclusive(true);
    }
}

void MainWindow::displayFragmentMenuButton(){
    editFragmentMenuButton->setVisible(true);
    editFragmentMenuButton->setChecked(true);
}

void MainWindow::hideFragmentMenuButton(){
    editFragmentMenuButton->setVisible(false);
}

void MainWindow::initFileActions(){
    fileActionGroup = new QActionGroup(this);

    QAction *openJsonFileAction = new QAction("Open mandible JSON", this);
    connect(openJsonFileAction, &QAction::triggered, this, &MainWindow::openMandJSON);

    QAction *openJsonFibFileAction = new QAction("Open fibula JSON", this);
    connect(openJsonFibFileAction, &QAction::triggered, this, &MainWindow::openFibJSON);

    QAction *cutMeshAction = new QAction("Cut", this);
    connect(cutMeshAction, &QAction::triggered, skullViewer, &Viewer::cutMesh);
    // connect(cutMeshAction, &QAction::triggered, this, &MainWindow::displayFragmentMenuButton);

    QAction *uncutMeshAction = new QAction("Undo cut", this);
    connect(uncutMeshAction, &QAction::triggered, skullViewer, &Viewer::uncutMesh);
    connect(uncutMeshAction, &QAction::triggered, fibulaViewer, &Viewer::uncutMesh);
    // connect(uncutMeshAction, &QAction::triggered, this, &MainWindow::hideFragmentMenuButton);

    fileActionGroup->addAction(openJsonFileAction);
    fileActionGroup->addAction(openJsonFibFileAction);
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

    editMenuButton = new QPushButton("Edit Graphics", this);
    connect(editMenuButton, &QPushButton::clicked, this, &MainWindow::displayEditMenu);
    editMenuButton->setCheckable(true);
    editMenuButton->setChecked(false);

    editFragmentMenuButton = new QPushButton("Edit Fragments", this);
    connect(editFragmentMenuButton, &QPushButton::clicked, this, &MainWindow::displayEditFragmentMenu);
    editFragmentMenuButton->setCheckable(true);
    editFragmentMenuButton->setChecked(false);
    //editFragmentMenuButton->setVisible(false);

    fileToolBar->addWidget(editMenuButton);
    fileToolBar->addWidget(editFragmentMenuButton);

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



