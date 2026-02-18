//Paso #16. Crear la funcion del paso #15
void MainWindow::leerImagenesCamara(){
    //Paso #20. Leer la imagen de la camara
    camara >> imagenPrincipal;
    //Paso #22. Si la imagen esta vacia, termino el programa
    if(imagenPrincipal.empty()) return;

    //Paso #23. Redimensionar la imagen
    cv::resize(imagenPrincipal, imagenChica, cv::Size(800,600));

    //Paso #24. Convertir la imagen del #23 a una imagen QImage, usando la funcion del paso #7
    QImage imagenQT = MatToQImage(imagenChica);

    //Paso #25. Convertir QImage a QPixmap
    QPixmap mapaPixeles = QPixmap::fromImage(imagenQT);

    //Paso #26. Visualizar el resultado en un label
    ui->label->setPixmap(mapaPixeles);
}
