const baseWriteUrl = "https://api.thingspeak.com/update?api_key=";

const calibrateField = "&field3=";
let calibrateWriteStatus = "0";

const baseReadUrl = "https://api.thingspeak.com/channels/";

const timeVeryfing = 60; //time in seconds

let readlLastStateUrl = "/fields/3/last.json?timezone=America%2FArgentina%2FBuenos_Aires";

let inputWriteApiKey = document.querySelector("#inputWriteApiKey");

let inputChannel = document.querySelector("#inputChannel");

let buttonCalibrate = document.querySelector("#buttonCalibrate");

let mensajes = document.querySelector("#mensajes");

buttonCalibrate.addEventListener('click', validateCal);

function validateCal() { 
    if (document.querySelector("#inputWriteApiKey").value !== "" && document.querySelector("#inputChannel").value !== "") {
        sendCalibrationRequest();
    } else {
        mensajes.innerHTML = "<p class='fail'>Para la calibración remota, los campos de Write API Key y ID Canal Thingspeak son obligatorios</p>";
        console.log("No completados los inputs inputWriteApiKey, inputChannel");
    }
}

async function sendCalibrationRequest(){
    
    let inputWriteApiKeyValue = inputWriteApiKey.value;
    calibrateWriteStatus = "1";
    let writeUrl = baseWriteUrl +  inputWriteApiKeyValue + calibrateField + calibrateWriteStatus;
    verifyCalibrationStateDelayed();

    try {
        let res = await fetch(writeUrl);
        let json = await res.json();
        console.log(json);
        console.log("Enviada solicitud de calibracion");
        console.log("Estado enviado: " + calibrateWriteStatus);
        } 
    catch (error) {
    console.log(error);
    }
}

async function sendCalibrationReset(){
    console.log("Reseteando solicitud de calibracion");
    let inputWriteApiKeyValue = inputWriteApiKey.value;
    calibrateWriteStatus = "0";
    let writeUrl = baseWriteUrl +  inputWriteApiKeyValue + calibrateField + calibrateWriteStatus;

    try {
        let res = await fetch(writeUrl);
        let json = await res.json();
        console.log(json);
        console.log(json.field3);
    
        } 
    catch (error) {
    console.log(error);
    }
    
}


async function verifyCalibrationState(){

    let inputChannelValue = document.querySelector("#inputChannel").value;
    let ReadUrl = baseReadUrl + inputChannelValue + readlLastStateUrl;
    
    try {

        let res = await fetch(ReadUrl);
        let json = await res.json();
        console.log(json);
        console.log(json.field3);
        let device_state = json.field3;
        if(json.field3==2){

            mensajes.innerHTML = "<p class='success'>La petición de calibración fue recibida con exito, el medidor ha iniciado la calibración remota</p>";
            console.log("La petición de calibración fue recibida, estado: " + device_state);
        }
        else {
            mensajes.innerHTML = "<p class='fail'>Error en la recepción de calibración, el medidor NO ha podido iniciar la calibración remota. Asegurese que el medidor esta encendido y con conexión a una red. Luego vuelva a intertarlo más tarde</p>";
            console.log("La petición de calibración no fue recibida, estado: " + device_state);
            sendCalibrationReset();
        }

    } 
    catch (error) {
    console.log(error);
    }
}

function verifyCalibrationStateDelayed(){
    countDown(timeVeryfing);
}

function countDown(i) {
    let interval = setInterval(function() {
        mensajes.innerHTML = "<p>Aguardando respuesta del medidor. Procesando la petición de calibración ... " + i + "</p>";
       
        if (i === 0) {
            clearInterval(interval); 
            verifyCalibrationState();
            
        }
        else {
            i--;
        }
    }, 1000);
}
