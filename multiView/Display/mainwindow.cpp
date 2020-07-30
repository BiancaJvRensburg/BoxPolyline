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
    StandardCamera *sc = new StandardCamera();
    mandViewer = new Viewer(this, sc, sliderMax);


    // The fibula viewer
    StandardCamera *scFibula = new StandardCamera();
    fibulaViewer = new ViewerFibula(this, scFibula, sliderMax, fibulaOffsetMax);

    // Main widget
    QWidget *mainWidget = new QWidget(this);

    // Horizontal layout
    QHBoxLayout *windowLayout = new QHBoxLayout();

    // Add the viewer to the layout
    windowLayout->addWidget(mandViewer);
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

    mandDockWidget = new QDockWidget("Plane controls");
    mandDockWidget->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
    mandDockWidget->setVisible(false);

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

    connect(leftPlaneSlider, static_cast<void (QSlider::*)(int)>(&QSlider::sliderMoved), mandViewer, &Viewer::moveLeftPlane);
    connect(rightPlaneSlider, static_cast<void (QSlider::*)(int)>(&QSlider::sliderMoved), mandViewer, &Viewer::moveRightPlane);

    QSlider *rotatePolylineMandible = new QSlider(Qt::Horizontal);
    rotatePolylineMandible->setMaximum(360);
    contentLayoutFibula->addRow("Fibula orientation", rotatePolylineMandible);

    connect(rotatePolylineMandible, static_cast<void (QSlider::*)(int)>(&QSlider::sliderMoved), mandViewer, &Viewer::rotatePolylineOnAxis);
    connect(mandViewer, &Viewer::toRotatePolylineOnAxis, fibulaViewer, &ViewerFibula::rotatePolylineOnAxisFibula);

    QSlider *fibulaOffsetSlider = new QSlider(Qt::Horizontal);
    fibulaOffsetSlider->setMaximum(100);
    contentLayoutFibula->addRow("Fibula offset", fibulaOffsetSlider);
    connect(fibulaOffsetSlider, static_cast<void (QSlider::*)(int)>(&QSlider::sliderMoved), fibulaViewer, &ViewerFibula::slidePolyline);

    layout->addWidget(contentsMand);
    layout->addWidget(contentsFibula);

    QWidget* controlWidget = new QWidget();
    controlWidget->setLayout(layout);

    mandDockWidget->setWidget(controlWidget);

    this->addDockWidget(Qt::BottomDockWidgetArea, mandDockWidget);
}

void MainWindow::initEditMenu(){
    editMenuWidget = new QDockWidget("Edit Menu");
    editMenuWidget->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);

    QVBoxLayout* layout = new QVBoxLayout();

    QPushButton *toggleDrawPlanesButton = new QPushButton(tr("&Draw planes"));
    toggleDrawPlanesButton->setCheckable(true);
    toggleDrawPlanesButton->setChecked(true);

    connect(toggleDrawPlanesButton, &QPushButton::released, mandViewer, &Viewer::toggleDrawPlane);
    connect(toggleDrawPlanesButton, &QPushButton::released, fibulaViewer, &ViewerFibula::toggleDrawPlane);

    QPushButton *toggleDrawCurveButton = new QPushButton(tr("&Draw curve"));
    toggleDrawCurveButton->setCheckable(true);
    toggleDrawCurveButton->setChecked(false);

    connect(toggleDrawCurveButton, &QPushButton::released, mandViewer, &Viewer::toggleDrawCurve);
    connect(toggleDrawCurveButton, &QPushButton::released, fibulaViewer, &ViewerFibula::toggleDrawCurve);

    // Adjust the plane transparency
    QGroupBox *sliderBox = new QGroupBox("Transparencies", this);
    QHBoxLayout *sliderBoxLayout = new QHBoxLayout();

    QSlider *planeAlphaSlider = new QSlider(Qt::Vertical);
    planeAlphaSlider->setMaximum(100);
    planeAlphaSlider->setSliderPosition(50);
    connect(planeAlphaSlider, static_cast<void (QSlider::*)(int)>(&QSlider::sliderMoved), mandViewer, &Viewer::setPlaneAlpha);
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
    connect(meshAlphaSlider, static_cast<void (QSlider::*)(int)>(&QSlider::sliderMoved), mandViewer, &Viewer::setMeshAlpha);
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

    connect(mandViewer, &Viewer::enableFragmentEditing, this, &MainWindow::enableFragmentEditing);
    connect(mandViewer, &Viewer::disableFragmentEditing, this, &MainWindow::disableFragmentEditing);

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

    connect(radioFrag1, &QRadioButton::toggled, this, &MainWindow::toEditBoxCentre);
    connect(radioFrag2, &QRadioButton::toggled, this, &MainWindow::toEditBoxStart);
    connect(radioFrag3, &QRadioButton::toggled, this, &MainWindow::toEditBoxEnd);
    connect(radioFragPlanes, &QPushButton::toggled, this, &MainWindow::toEditPlane);

    QVBoxLayout *vbox = new QVBoxLayout;

    vbox->addWidget(radioFragPlanes);
    vbox->addWidget(radioFrag1);
    vbox->addWidget(radioFrag2);
    vbox->addWidget(radioFrag3);
    vbox->addStretch(1);
    groupRadioBox->setLayout(vbox);

    connect(mandViewer, &Viewer::editPlane, this, &MainWindow::editPlane);
    connect(mandViewer, &Viewer::editBoxCentre, this, &MainWindow::editBoxCentre);

    connect(groupRadioBox, &QGroupBox::clicked, this, &MainWindow::setFragRadios);

    toggleDrawMeshButton = new QPushButton(tr("&Draw mesh"));
    toggleDrawMeshButton->setCheckable(true);
    toggleDrawMeshButton->setChecked(false);

    connect(toggleDrawMeshButton, &QPushButton::released, mandViewer, &Viewer::toggleDrawMesh);

    QPushButton *drawPolylineButton = new QPushButton("Draw Polyline", this);
    connect(drawPolylineButton, &QPushButton::clicked, mandViewer, &Viewer::toggleDrawPolyline);
    connect(drawPolylineButton, &QPushButton::clicked, fibulaViewer, &ViewerFibula::toggleDrawPolyline);

    QPushButton *drawBoxesButton = new QPushButton("Draw Boxes", this);
    connect(drawBoxesButton, &QPushButton::clicked, mandViewer, &Viewer::toggleDrawBoxes);
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
        //radioFragPlanes->setChecked(true);
    }
}

void MainWindow::displayFragmentMenuButton(){
    /*editFragmentMenuButton->setVisible(true);
    editFragmentMenuButton->setChecked(true);*/
   //skullDockWidget->setVisible(false);
    currentPlane = 0;
    currentBox = 1;
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

    // LINK THE VIEWERS
    connect(mandViewer, &Viewer::constructPoly, fibulaViewer, &ViewerFibula::constructPolyline);
    connect(fibulaViewer, &ViewerFibula::okToPlacePlanes, mandViewer, &Viewer::placePlanes);
    connect(mandViewer, &Viewer::toUpdateDistances, fibulaViewer, &ViewerFibula::updateDistances);
    connect(mandViewer, &Viewer::toUpdatePlaneOrientations, fibulaViewer, &ViewerFibula::updatePlaneOrientations);
    connect(mandViewer, &Viewer::toRotatePolylineOnAxis, fibulaViewer, &ViewerFibula::rotatePolylineOnAxisFibula);
    connect(mandViewer, &Viewer::planeMoved, fibulaViewer, &ViewerFibula::movePlanes);
    connect(fibulaViewer, &ViewerFibula::sendToManible, mandViewer, &Viewer::recieveFromFibulaMesh);
    connect(fibulaViewer, &ViewerFibula::requestNewNorms, mandViewer, &Viewer::sendNewNorms);
    connect(mandViewer, &Viewer::cutFibula, fibulaViewer, &ViewerFibula::cut);
    connect(mandViewer, &Viewer::uncutFibula, fibulaViewer, &ViewerFibula::uncut);
    connect(mandViewer, &Viewer::toReinitBox, fibulaViewer, &ViewerFibula::reinitBox);
    connect(mandViewer, &Viewer::toReinitPoly, fibulaViewer, &ViewerFibula::reinitPoly);
    connect(mandViewer, &Viewer::polylineBent, fibulaViewer, &ViewerFibula::bendPolylineNormals);
    connect(fibulaViewer, &ViewerFibula::requestFakeBend, mandViewer, &Viewer::fakeBend);
}

void MainWindow::initFileMenu(){
    initFileActions();

    /*QMenu *fileMenu = menuBar()->addMenu(tr("File"));
    fileMenu->addActions(fileActionGroup->actions());*/
}

void MainWindow::initToolBars () {
    QToolBar *fileToolBar = new QToolBar(this);
    fileToolBar->addActions(fileActionGroup->actions());

    loadedMeshes = new QDockWidget("Cutting Controls");
    loadedMeshes->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
    loadedMeshes->setVisible(false);
    QWidget *controlWidget = new QWidget();
    QHBoxLayout *loadedMeshesLayout = new QHBoxLayout();

    QPushButton *cutMeshAction = new QPushButton("Cut", this);
    connect(cutMeshAction, &QPushButton::clicked, mandViewer, &Viewer::cutMesh);
    connect(cutMeshAction, &QPushButton::clicked, this, &MainWindow::displayFragmentMenuButton);

    QPushButton *uncutMeshAction = new QPushButton("Undo Cut", this);
    connect(uncutMeshAction, &QPushButton::clicked, mandViewer, &Viewer::uncutMesh);
    connect(uncutMeshAction, &QPushButton::clicked, fibulaViewer, &Viewer::uncutMesh);
    connect(uncutMeshAction, &QPushButton::clicked, this, &MainWindow::hideFragmentMenuButton);

    editMenuButton = new QPushButton("Edit Graphics", this);
    connect(editMenuButton, &QPushButton::clicked, this, &MainWindow::displayEditMenu);
    editMenuButton->setCheckable(true);
    editMenuButton->setChecked(false);

    QPushButton *pandaButton = new QPushButton("Set panda", this);
    connect(pandaButton, &QPushButton::clicked, fibulaViewer, &ViewerFibula::setPanda);

    QPushButton *pandaResultButton = new QPushButton("Print result", this);
    connect(pandaResultButton, &QPushButton::clicked, fibulaViewer, &ViewerFibula::printPandaResults);

    QPushButton *pandaNextButton = new QPushButton("Next plane", this);
    connect(pandaNextButton, &QPushButton::clicked, fibulaViewer, &ViewerFibula::nextPandaPlane);

    QPushButton *pandaPrevButton = new QPushButton("Prev plane", this);
    connect(pandaPrevButton, &QPushButton::clicked, fibulaViewer, &ViewerFibula::prevPandaPlane);

    editFragmentMenuButton = new QPushButton("Edit Fragments", this);
    connect(editFragmentMenuButton, &QPushButton::clicked, this, &MainWindow::displayEditFragmentMenu);
    editFragmentMenuButton->setCheckable(true);
    editFragmentMenuButton->setChecked(false);

    loadedMeshesLayout->addWidget(cutMeshAction);
    loadedMeshesLayout->addWidget(uncutMeshAction);
    loadedMeshesLayout->addWidget(editFragmentMenuButton);
    loadedMeshesLayout->addWidget(pandaButton);
    loadedMeshesLayout->addWidget(pandaNextButton);
    loadedMeshesLayout->addWidget(pandaPrevButton);
    loadedMeshesLayout->addWidget(pandaResultButton);
    controlWidget->setLayout(loadedMeshesLayout);
    loadedMeshes->setWidget(controlWidget);

    fileToolBar->addWidget(editMenuButton);
    //fileToolBar->addWidget(loadedMeshes);

    this->addDockWidget(Qt::TopDockWidgetArea, loadedMeshes);

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

// A plane is selected in the mandible viewer
void MainWindow::editPlane(unsigned int i){
    currentPlane = i;
    groupRadioBox->setChecked(true);
    radioFragPlanes->setChecked(true);
    toEditPlane(false);
    mandViewer->toggleEditPlaneMode(i, true);
}

// A bone fragment is selected in the mandible viewer
void MainWindow::editBoxCentre(unsigned int i){
    currentBox = i;
    groupRadioBox->setChecked(true);
    radioFrag1->setChecked(true);
    toEditBoxCentre(false);
    mandViewer->toggleEditBoxMode(i, true);
}

void MainWindow::editBoxStart(unsigned int i){
    currentBox = i;
    groupRadioBox->setChecked(true);
    radioFrag2->setChecked(true);
    toEditBoxStart(false);
    mandViewer->toggleEditFirstCorner(i, true);
}

void MainWindow::editBoxEnd(unsigned int i){
    currentBox = i;
    groupRadioBox->setChecked(true);
    setFragRadios();
    radioFrag3->setChecked(true);
    toEditBoxEnd(false);
    mandViewer->toggleEditEndCorner(i, true);
}

void MainWindow::toEditPlane(bool b){
    if(!b) mandViewer->toggleAllPlanes(b);
    else mandViewer->toggleEditPlaneMode(currentPlane, b);
}

void MainWindow::toEditBoxCentre(bool b){
    if(!b) mandViewer->toggleAllBoxes(b);
    else mandViewer->toggleEditBoxMode(currentBox, b);
}

void MainWindow::toEditBoxStart(bool b){
    if(!b) mandViewer->toggleAllFirstCorners(b);
    else mandViewer->toggleEditFirstCorner(currentBox, b);
}

void MainWindow::toEditBoxEnd(bool b){
    if(!b) mandViewer->toggleAllEndCorners(b);
    else mandViewer->toggleEditEndCorner(currentBox, b);
}

void MainWindow::openMandJSON(){
    bool isOpen = openJSON(mandViewer);
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
        loadedMeshes->setVisible(true);
        mandDockWidget->setVisible(true);
        update();
    }
}



