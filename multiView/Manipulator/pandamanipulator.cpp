#include "pandamanipulator.h"

PandaManipulator::PandaManipulator()
{
    mouse_released = true;
    RepX = qglviewer::Vec(1,0,0);
    RepY = qglviewer::Vec(0,1,0);
    RepZ = qglviewer::Vec(0,0,1);
    display_scale = 1.;
    mode_modification = 0.f;
    Xscale = Yscale = Zscale = 1.;
    mode_grabbing = 1;
    id = 0;
}

void PandaManipulator::checkIfGrabsMouse(int x, int y, const qglviewer::Camera *const cam){
    qglviewer::Vec src;
    qglviewer::Vec img ;

    double lambda , epsilon_rotation_detect = 0.2 , epsilon_tranlation_detect = 0.005;
    qglviewer::Vec eye,dir;
    qglviewer::Vec Eye , X , Z;
    qglviewer::Vec Dir , d , e;

    //Ce test marche, et permet maintenant de manipuler une sphre dans un gl, et un modle dans l'autre, ou la sphre de
    //l'autre gl en mme temps ... parfait :-) ...

    // Liste des tats successifs de la sphere:
    // 0 : absente -> alors setGrabsMouse(false);
    // 1 : positionning -> true; et on la dirige sur la surface du maillage.
    // 2 : (positionned and) directing -> true; son centre est fix, on la dirige.
    // 3 : fixed -> manhattanLength() <= 50 px; on devra rappuyer sur le bouton crer une sphre ou appuyer sur clic droit

    // Clic gauche : passer  l'tat suivant
    // Clic droit : reprendre l'tat prcdent
    switch(state)
    {
    case 0:
        setGrabsMouse(false);
        // Dans cet tat, la sphre est dsactive
        break;

    case 1:
        // Dans cet tat, on vrifie si l'utilisateur a cliqu dans une zone d'influence du SimpleManipulator
        if( ! mouse_released )
        {
            setGrabsMouse(true);
            return;
        }

        if( mode_grabbing == 1 )
        {
            cam->convertClickToLine(QPoint(x,y),eye,dir);
            Eye = qglviewer::Vec(eye[0],eye[1],eye[2]);
            Dir = qglviewer::Vec(dir[0],dir[1],dir[2]);

            ///////////////////////////////////////  Translations:   ///////////////////////////////////////

            // Check on ty :
            d = cross( RepY , Dir );
            e = cross( Dir , d );
            lambda = ( ( Eye - Origin )*( e ) ) / ( RepY * e );
            X = Origin + lambda*RepY;
            if( lambda < 2.2*display_scale && lambda > -2.2*display_scale )
            {
                Z = Eye + ( Dir * ( X-Eye ) )*Dir/sqrt( (Dir*Dir) );
                if( ( ( Z-X )*( Z-X ) ) < display_scale*display_scale*epsilon_tranlation_detect )
                {
                    mode_modification = 2;
                    setGrabsMouse(true);
                    return;
                }
            }

            // Check on tz :
            d = cross( RepZ , Dir );
            e = cross( Dir , d );
            lambda = ( ( Eye - Origin )*( e ) ) / ( RepZ * e );
            X = Origin + lambda*RepZ;
            if( lambda < 2.2*display_scale && lambda > -2.2*display_scale )
            {
                Z = Eye + ( Dir * ( X-Eye ) )*Dir/sqrt( (Dir*Dir) );
                if( ( ( Z-X )*( Z-X ) ) < display_scale*display_scale*epsilon_tranlation_detect )
                {
                    mode_modification = 3;
                    setGrabsMouse(true);
                    return;
                }
            }

            mode_modification = 0;
            setGrabsMouse(false);
            break;
        }
    }
}

void PandaManipulator::draw()
{
    glPolygonMode( GL_FRONT_AND_BACK , GL_FILL );
    if(state == 1)
    {
       if( mode_grabbing == 1 )
            {
                // Dans BLENDER, ils affichent un modle de manipulateur diffrent suivant : mode_modification , mouse_released.

            float CX[3] = {0.f , 1.f , 0.f};
            float CY[3] = {0.f , 0.f , 1.f};
            float CZ[3] = {1.f , 0.f , 0.f};
            float Selection[3] = {1.f , 1.f , 0.f};

            glDisable(GL_LIGHTING);

            glLineWidth( 2.f );
            qglviewer::Vec p;
            glBegin( GL_LINES );

            if(mode_modification == 2)
                glColor3fv( Selection );
            else
                glColor3fv( CY );

            p = Origin - 2 * display_scale * RepY;
            glVertex3f(p[0],p[1],p[2]);
            // p = Origin + 2 * display_scale * RepY;
            p = Origin;
            glVertex3f(p[0],p[1],p[2]);

            if(mode_modification == 3)
                glColor3fv( Selection );
            else
                glColor3fv( CZ );

            p = Origin - 2 * display_scale * RepZ;
            glVertex3f(p[0],p[1],p[2]);
            //p = Origin + 2 * display_scale * RepZ;
            p = Origin;
            glVertex3f(p[0],p[1],p[2]);
            glEnd();

            glEnable(GL_LIGHTING);
        }
    }
}

void PandaManipulator::clear()
{
    this->coordinates.clear();
    this->setState( 0 );
    this->idpoints.clear();
}

void PandaManipulator::manipulatedCallback()
{
    Q_EMIT moved(Origin);
}

void PandaManipulator::fakeMouseDoubleClickEvent( QMouseEvent* const )
{
    if( mode_modification == 7 || mode_modification == -7 )
    {
        Xscale = 1.f;
        manipulatedCallback();
        return;
    }
    if( mode_modification == 8 || mode_modification == -8 )
    {
        Yscale = 1.f;
        manipulatedCallback();
        return;
    }
    if( mode_modification == 9 || mode_modification == -9 )
    {
        Zscale = 1.f;
        manipulatedCallback();
        return;
    }
}

void PandaManipulator::mousePressEvent( QMouseEvent* const event  , qglviewer::Camera* const cam )
{
    mouse_released = false;

        if( mode_modification > 3 || mode_modification < -3 )
        {
            return;
        }

        double lambda;
        qglviewer::Vec eye,dir;
        qglviewer::Vec Eye;
        qglviewer::Vec Ur,Vr,Dir , d , e;
        cam->convertClickToLine(event->pos(),eye,dir);
        Eye = qglviewer::Vec(eye[0],eye[1],eye[2]);
        Dir = qglviewer::Vec(dir[0],dir[1],dir[2]);

        if( mode_modification == 2 )
        {
            d = cross( RepY , Dir );
            e = cross( Dir , d );
            lambda = ( ( Eye - Origin )*( e ) ) / ( RepY * e );
            PrevPos = Origin + lambda*RepY;
            return;
        }
        if( mode_modification == 3 )
        {
            d = cross( RepZ , Dir );
            e = cross( Dir , d );
            lambda = ( ( Eye - Origin )*( e ) ) / ( RepZ * e );
            PrevPos = Origin + lambda*RepZ;
            return;
        }
}

void PandaManipulator::mouseReleaseEvent( QMouseEvent* const , qglviewer::Camera* const  )
{
    mouse_released = true;
    Q_EMIT mouseReleased();
}

void PandaManipulator::mouseMoveEvent(QMouseEvent* const event, qglviewer::Camera* const cam)
{
    if( ! mouse_released )
    {

        if( mode_grabbing == 1 )
        {
            qglviewer::Vec eye,dir;
            qglviewer::Vec Eye , NewPos;
            qglviewer::Vec Dir , d , e ;
            qglviewer::Vec Ur , Vr;
            double lambda;

            cam->convertClickToLine(event->pos(),eye,dir);
            Eye = qglviewer::Vec(eye[0],eye[1],eye[2]);
            Dir = qglviewer::Vec(dir[0],dir[1],dir[2]);

            switch(mode_modification)
            {
            case 2:
                // Alors on doit trouver le point sur la droite (Origine,RepY) qui est le plus proche du rayon
                d = cross( RepY , Dir );
                e = cross( Dir , d );
                lambda = ( ( Eye - Origin ) * e ) / ( RepY * e );
                NewPos = Origin + lambda*RepY;
                Origin += NewPos - PrevPos;
                PrevPos = NewPos;
                manipulatedCallback();
                break;
            case 3:
                // Alors on doit trouver le point sur la droite (Origine,RepZ) qui est le plus proche du rayon
                d = cross( RepZ , Dir );
                e = cross( Dir , d );
                lambda = ( ( Eye - Origin ) * e ) / ( RepZ * e );
                NewPos = Origin + lambda*RepZ;
                Origin += NewPos - PrevPos;
                PrevPos = NewPos;
                manipulatedCallback();
                break;
            }
        }
    }
}


