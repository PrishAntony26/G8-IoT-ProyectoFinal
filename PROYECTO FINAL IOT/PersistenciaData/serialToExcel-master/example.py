from serialToExcel import SerialToExcel

serialToExcel = SerialToExcel("COM3",115200)

#columnas = ["Nro Lectura","Valor"]

serialToExcel.setColumns(["Nro Lectura","Humedad","Temperatura","Humedad Suelo"])
serialToExcel.setRecordsNumber(20)
serialToExcel.readPort()

serialToExcel.writeFile("archivo2.xls")