void MainWindow::on_pushButton_clicked()
{
    //Paso #27. Verificar si la imagen esta vacia
    if(imagenPrincipal.empty()) return;

    //Paso #28. Tomar una imagen
    cv::imwrite("imagen.jpg",imagenPrincipal);

}
