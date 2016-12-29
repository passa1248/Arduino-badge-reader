var net = require('net');
var HOST = '192.168.1.5';//'172.20.10.2'
var PORT = 11000;



var mysql = require('mysql');
 




net.createServer(function(sock) {
    
    // We have a connection - a socket object is assigned to the connection automatically
    console.log('CONNECTED: ' + sock.remoteAddress +':'+ sock.remotePort);

    
    // Add a 'data' event handler to this instance of socket
    sock.on('data', function(data) {
		data = data.toString().trim();
        if (data.length == 0)
		{
			return;
        }

        console.log(sock.remoteAddress + ' - ' + 'Request: ' + data);
	 
	 var response = "";
	 
	 var query = 'SELECT * FROM ANAGRAFICA_DIPENDENTI WHERE CODICE =  "' + data + '"';

	 
	var connection = mysql.createConnection(
    {
      host     : 'xxxxx',
      user     : 'xxxxx',
      password : 'xxxxx',
      database : 'xxxxx',
	  port     : xxxx
    }
);

connection.connect();
	 
 connection.query(query, function(err, results) {
     if (err) {
         response = '$0.Errore query.null&\r\n';
         console.log(sock.remoteAddress + ' - ' + 'Response: ' + response.toString().trim());
         sock.write(response);
		throw err;
	};
	if (results.length == 0)
    {

		response = '$0.Codice non presente!.null&\r\n';
        console.log(sock.remoteAddress + ' - ' + 'Response: ' + response.toString().trim());
     	sock.write(response);
				
		 
	}
	else
    {
        var nomeDipendente = results[0].NOME;
        var idDipendente = results[0].ID
        var currentDate = new Date();
        currentDate.setTime(currentDate.getTime() - new Date().getTimezoneOffset() * 60 * 1000);
        currentDate = currentDate.toISOString().slice(0, 19).replace('T', ' ');
		
        var query = 'SELECT * FROM TIMBRATURE WHERE DATE(GIORNO_LAVORO) = DATE(NOW()) AND ID_DIPENDENTE = ' + idDipendente;
      //  console.log(query);

     connection.query(query, function (err, results) {
         if (err) {
             response = '$0.Errore query!.null&\r\n';
             console.log(sock.remoteAddress + ' - ' + 'Response: ' + response.toString().trim());
             sock.write(response);
			  
             throw err;
         };
         if (results.length != 0)
         { // HA GIA TIMBRATO UNA VOLTA
   
             var COLONNA = '';
             var idRow = results[0].ID;
             
             if (results[0].ENTRATA == null) {
                 COLONNA = 'ENTRATA';
                 response = '$1.Buongiorno ' + nomeDipendente + '.' + currentDate + '&\r\n';
             }
             else if (results[0].INIZIO_PAUSA_PRANZO == null) {
                 COLONNA = 'INIZIO_PAUSA_PRANZO';
                 response = '$1.Buon appetito ' + nomeDipendente + '.' + currentDate + '&\r\n';
             }
             else if (results[0].FINE_PAUSA_PRANZO == null) {
                 COLONNA = 'FINE_PAUSA_PRANZO';
                 response = '$1.Buon pomeriggio ' + nomeDipendente + '.' + currentDate + '&\r\n';
             }
             else if (results[0].USCITA == null) {
                 COLONNA = 'USCITA';
                 response = '$1.Buona serata ' + nomeDipendente + '.' + currentDate + '&\r\n';
             }

             if (COLONNA == '')
             {
                 response = '$0.Hai gia timbrato!.null&\r\n';
                 console.log(sock.remoteAddress + ' - ' + 'Response: ' + response.toString().trim());
                 sock.write(response);
			
                 return;
             }

             query = 'UPDATE TIMBRATURE SET ' + COLONNA + ' = ' + '"' + currentDate + '"  WHERE ID = ' + idRow;

        //     console.log(query);

             connection.query(query, function (err, results) {
                 if (err) {
                     response = '$0.Errore query.null&\r\n';
                     console.log(sock.remoteAddress + ' - ' + 'Response: ' + response.toString().trim());
                     sock.write(response);
                     throw err;
                     
                 }
             });

           	
             console.log(sock.remoteAddress + ' - ' + 'Response: ' + response.toString().trim());
             sock.write(response);
            
             return;

         }// end if
         else { // PRIMA TIMBRATIRA DEL GIORNO ****************************************


               query = 'INSERT INTO TIMBRATURE (ID_DIPENDENTE, GIORNO_LAVORO, ENTRATA) VALUES(' + idDipendente + ',"' + currentDate.substring(0,10) + '","' + currentDate  + '");';
        //       console.log(query);
               connection.query(query, function (err, results) {
                   if (err)
                   {                 
                       response = '$0.Errore query.null&\r\n';
                       console.log(sock.remoteAddress + ' - ' + 'Response: ' + response.toString().trim());
                       sock.write(response);
                       throw err;
                   };


                   response = '$1.Buongiorno ' + nomeDipendente + '.' + currentDate + '&\r\n';
                   console.log(sock.remoteAddress + ' - ' + 'Response: ' + response.toString().trim());
                   sock.write(response);
				  

               })//end connection query
     
	 
		
          } // end else
    })// end connection query
 }// end else
 
   
     }); // end connection query
	
    }); // end event on data
    sock.on('end', function(){
	
    sock.end();
    });

    sock.on('close', function(data) {
        console.log('CLOSED: ' + sock.remoteAddress +':'+ sock.remotePort);
    });
    sock.on('error', function(err) {
        console.log(err);
    });
    
}).listen(PORT, HOST);

console.log('Server listening on ' + HOST +':'+ PORT);