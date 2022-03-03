//Urls for reading and writing in thingspeak API

const baseWriteUrl = "https://api.thingspeak.com/update?api_key=";

const baseReadUrl = "https://api.thingspeak.com/channels/";

const readlLastStateUrl = "last.json?timezone=America%2FArgentina%2FBuenos_Aires";

const calibrateField = "3";

const resetField = "4";

const timesetField = "5";

//Configuration Parameters
const timeVeryfing = 60; //time in seconds

let calibrateWriteStatus = "0";

let restartWriteStatus = "0";

let timesetWriteStatus = "0";

let remoteOperation = "";


//DOM interaction
let inputWriteApiKey = document.querySelector("#inputWriteApiKey");

let inputChannel = document.querySelector("#inputChannel");

let buttonCalibrate = document.querySelector("#buttonCalibrate");

let buttonRestart = document.querySelector("#buttonRestart");

buttonRestart.addEventListener('click', validateRestart);

let buttonTimeset = document.querySelector("#buttonTimeset");

buttonTimeset.addEventListener('click', validateTimeSet);

let mensajes = document.querySelector("#mensajes");

buttonCalibrate.addEventListener('click', validateCal);


//Operations Functions

//Calibration

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
    let writeUrl = baseWriteUrl +  inputWriteApiKeyValue + "&field" + calibrateField + "=" + calibrateWriteStatus;
    console.log(writeUrl);
    verifyCalibrationStateDelayed();

    try {
        let res = await fetch(writeUrl);
        let json = await res.json();
        console.log(json);
        console.log("Enviando solicitud de calibracion...");
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
    let writeUrl = baseWriteUrl +  inputWriteApiKeyValue + "&field" + calibrateField + "=" + calibrateWriteStatus;

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
    let ReadUrl = baseReadUrl + inputChannelValue + "/fields/3/" + readlLastStateUrl;
    
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
    remoteOperation = "calibrate";
    countDown(timeVeryfing, remoteOperation);
}


//Restart

function validateRestart() { 
    if (document.querySelector("#inputWriteApiKey").value !== "" && document.querySelector("#inputChannel").value !== "") {
        sendRestartRequest();
    } else {
        mensajes.innerHTML = "<p class='fail'>Para el reinicio remoto, los campos de Write API Key y ID Canal Thingspeak son obligatorios</p>";
        console.log("No completados los inputs inputWriteApiKey, inputChannel");
    }
}


async function verifyRestartState(){

    let inputChannelValue = document.querySelector("#inputChannel").value;
    let ReadUrl = baseReadUrl + inputChannelValue + "/fields/4/" + readlLastStateUrl;
    
    try {

        let res = await fetch(ReadUrl);
        let json = await res.json();
        console.log(json);
        console.log(json.field4);
        let device_state = json.field4;
        if(json.field4==2){

            mensajes.innerHTML = "<p class='success'>La petición de reinicio fue recibida con exito, el medidor ha iniciado el reinicio remoto</p>";
            console.log("La petición de reinicio fue recibida, estado: " + device_state);
        }
        else {
            mensajes.innerHTML = "<p class='fail'>Error en la recepción de reinicio, el medidor NO ha podido ejecutar el reinicio remoto. Asegurese que el medidor esta encendido y con conexión a una red. Luego vuelva a intertarlo más tarde</p>";
            console.log("La petición de reinicio no fue recibida, estado: " + device_state);
            sendRestartReset();
        }

    } 
    catch (error) {
    console.log(error);
    }
}


async function sendRestartReset(){
    console.log("Anulando solicitud de reinicio");
    let inputWriteApiKeyValue = inputWriteApiKey.value;
    restartWriteStatus = "0";
    let writeUrl = baseWriteUrl +  inputWriteApiKeyValue + "&field" + resetField + "=" + restartWriteStatus;

    try {
        let res = await fetch(writeUrl);
        let json = await res.json();
        console.log(json);
        console.log(json.field4);
    
        } 
    catch (error) {
    console.log(error);
    }
    
}

async function sendRestartRequest(){
    
    let inputWriteApiKeyValue = inputWriteApiKey.value;
    restartWriteStatus = "1";
    let writeUrl = baseWriteUrl +  inputWriteApiKeyValue + "&field" + resetField + "=" + restartWriteStatus;
    console.log(writeUrl);
    verifyRestartStateDelayed();

    try {
        let res = await fetch(writeUrl);
        let json = await res.json();
        console.log(json);
        console.log("Enviando solicitud de reinicio...");
        console.log("Estado enviado: " + restartWriteStatus);
        } 
    catch (error) {
    console.log(error);
    }
}

function verifyRestartStateDelayed(){
    remoteOperation = "restart";
    countDown(timeVeryfing, remoteOperation);
}

//Time Set

function validateTimeSet() { 
    if (document.querySelector("#inputWriteApiKey").value !== "" && document.querySelector("#inputChannel").value !== "") {
        sendTimeSetRequest();
    } else {
        mensajes.innerHTML = "<p class='fail'>Para el ajuste de hora, los campos de Write API Key y ID Canal Thingspeak son obligatorios</p>";
        console.log("No completados los inputs inputWriteApiKey, inputChannel");
    }
}

async function sendTimeSetRequest(){
    
    let inputWriteApiKeyValue = inputWriteApiKey.value;
    timesetWriteStatus = "1";
    let writeUrl = baseWriteUrl +  inputWriteApiKeyValue + "&field" + timesetField + "=" + timesetWriteStatus;
    console.log(writeUrl);
    verifyTimeSetStateDelayed();

    try {
        let res = await fetch(writeUrl);
        let json = await res.json();
        console.log(json);
        console.log("Enviando solicitud de ajuste de hora...");
        console.log("Estado enviado: " + timesetWriteStatus);
        } 
    catch (error) {
    console.log(error);
    }
}

async function sendTimeSetReset(){
    console.log("Reseteando solicitud de ajuste de hora");
    let inputWriteApiKeyValue = inputWriteApiKey.value;
    timesetWriteStatus = "0";
    let writeUrl = baseWriteUrl +  inputWriteApiKeyValue + "&field" + timesetField + "=" + timesetWriteStatus;

    try {
        let res = await fetch(writeUrl);
        let json = await res.json();
        console.log(json);
        console.log(json.field5);
    
        } 
    catch (error) {
    console.log(error);
    }
    
}


async function verifyTimeSetState(){

    let inputChannelValue = document.querySelector("#inputChannel").value;
    let ReadUrl = baseReadUrl + inputChannelValue + "/fields/5/" + readlLastStateUrl;
    
    try {

        let res = await fetch(ReadUrl);
        let json = await res.json();
        console.log(json);
        console.log(json.field5);
        let device_state = json.field5;
        if(json.field5==2){

            mensajes.innerHTML = "<p class='success'>La petición de ajuste de hora fue recibida con exito, el medidor ha iniciado el ajuste de hora remoto</p>";
            console.log("La petición de ajuste de hora fue recibida, estado: " + device_state);
        }
        else {
            mensajes.innerHTML = "<p class='fail'>Error en la recepción de ajuste de hora, el medidor NO ha podido iniciar el ajuste de hora remoto. Asegurese que el medidor esta encendido y con conexión a una red. Luego vuelva a intertarlo más tarde</p>";
            console.log("La petición de ajuste de hora no fue recibida, estado: " + device_state);
            sendTimeSetReset();
        }

    } 
    catch (error) {
    console.log(error);
    }
}


function verifyTimeSetStateDelayed(){
    remoteOperation = "timeset";
    countDown(timeVeryfing, remoteOperation);
}


function countDown(i, operation) {
    let interval = setInterval(function() {
        mensajes.innerHTML = "<p>Aguardando respuesta del medidor. Procesando la petición ... " + i + "</p>";
       
        if (i === 0) {
            clearInterval(interval);

            if(operation === "calibrate"){
                verifyCalibrationState(); 
            }
            else if (operation === "restart"){
                verifyRestartState();
            }

            else if (operation === "timeset"){
                verifyTimeSetState();
            }
            
            
        }
        else {
            i--;
        }
    }, 1000);
}
