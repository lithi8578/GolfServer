#include "WiFiS3.h"
#include "avr/pgmspace.h"
const char *name="catalysd";
const char *passw="Fin20988";
IPAddress localIp(192,168,0,194);
IPAddress gateway(192,168,0,1);
IPAddress subnet(255,255,255,0);
IPAddress dns(8,8,8,8);
WiFiServer server(80);
String html PROGMEM=R"===(
  <!DOCTYPE html>
<html lang="zh-Hant">
<head>
  <meta charset="UTF-8" />
  <title>GOLF</title>
  <style>
    body {
      font-family: sans-serif;
      text-align: center;
      background: #eef2f7;
      margin: 0;
    }
    h1, h2 {
      margin: 10px;
    }
    .controls {
      display: flex;
      justify-content: center;
      gap: 10px;
      flex-wrap: wrap;
      margin: 10px;
    }
    label {
      font-weight: bold;
    }
    input[type="number"] {
      width: 60px;
      padding: 4px;
      border-radius: 5px;
      border: 1px solid #aaa;
      text-align: center;
    }
    button {
      padding: 6px 14px;
      border: none;
      border-radius: 8px;
      background: #28a745;
      color: white;
      cursor: pointer;
    }
    button:hover {
      background: #218838;
    }
    #sign{
      height: 50px;
      width: 50px;
      border-radius: 30px;
      margin-left: 20px;
      background: #28a745;
    }
    canvas {
      display: block;
      margin: 20px auto;
      background: #2980b9;
      border-radius: 8px;
    }
  </style>
</head>
<body>
  <h1 style="display: inline;">GOLF 撞球遊戲</h1>
  <button id="cnct_state" style="background-color: red;padding: 7px 7px; "></button>
  <h2 id="content">輸入角度與力道再按「SHOT!」</h2>

  <div class="controls">
    <div>
      <label for="force">力道</label><br>
      <input id="force" type="number" min="0" max="100" value="0">
    </div>
    <div>
      <label for="angle">角度</label><br>
      <input id="angle" type="number" min="0" max="360" value="45">
    </div>
    <div >
      <label for="istrig"></label><br>
      <button id="istrig" type="button">SHOT!</button>
    </div>
    <div style="align-self: end;">
      <button id="sign" typy="button"></button>
    </div>
  </div>

  <canvas id="game" width="1200" height="500"></canvas>

  <script src="script.js"></script>
</body>
</html>
)===";
String css PROGMEM=R"===()===";
String js PROGMEM=R"===(
  const canvas = document.getElementById("game");
    const ctx = canvas.getContext("2d");
    const initialBallPosition = { x: 300, y: 250 };  
    const ball = {
      x: initialBallPosition.x,
      y: initialBallPosition.y,
      radius: 10,
      dx: 0,
      dy: 0,
    };

    let hole = {
      x: 1050,
      y: 250,
      radius: 15
    };

    const obstacles = [
      { x: 800, y: 100, w: 20, h: 100, dir: 1 },
      { x: 500, y: 300, w: 20, h: 100, dir: -1 },
    ];
    let displayDx = 0;   
    let displayDy = 0;   
    let displaySpeed = 0;
    let displayAngle = 0;
    const friction = 0.98;
    let isShooting = false;
    let mouseStart = null;
    let mouseCurrent = null;
    let isDragging = false;
    let online=false;
    var shotBtn= document.getElementById('istrig');
    var iptForce=document.getElementById('force');
    var iptAngle=document.getElementById('angle');
    var sign=document.getElementById('sign');
    var cnctState=document.getElementById('cnct_state');
    var theta=0;
    shotBtn.addEventListener('click',()=>{
      if(isShooting==false){
        theta=(iptAngle.value/180)*Math.PI;
        ball.dx=iptForce.value*Math.cos(theta);
        ball.dy=iptForce.value*Math.sin(theta);
        sign.style.backgroundColor='gray';
        isShooting=true;
      }
    });
    iptForce.addEventListener('change',()=>{sign.style.backgroundColor='yellow';});
    iptAngle.addEventListener('change',()=>{sign.style.backgroundColor='yellow';});
    canvas.addEventListener("mousedown", e => {
      if (!isShooting) {
        isDragging = true;
        mouseStart = { x: e.offsetX, y: e.offsetY };
        mouseCurrent = { x: e.offsetX, y: e.offsetY };
      }
    });

    canvas.addEventListener("mousemove", e => {
      if (isDragging) {
        mouseCurrent = { x: e.offsetX, y: e.offsetY };
        const dx= (mouseStart.x - mouseCurrent.x).toFixed(2);
        const dy= (mouseStart.y - mouseCurrent.y).toFixed(2);
        sign.style.backgroundColor='yellow';
        displaySpeed = Math.sqrt(dx*dx + dy*dy).toFixed(2);
        displayAngle = Math.atan2(dy, dx) * (180 / Math.PI);
        displayAngle = displayAngle.toFixed(1); 
      }
    });

    canvas.addEventListener("mouseup", e => {
      if (isDragging) {
        const dx = mouseStart.x - e.offsetX;
        const dy = mouseStart.y - e.offsetY;
        ball.dx = dx * 0.1;
        ball.dy = dy * 0.1;
        displaySpeed = Math.sqrt(dx*dx + dy*dy).toFixed(2);
        isShooting = true;
        isDragging = false;
        sign.style.backgroundColor='gray';
      }
    });

    function drawBall() {
      ctx.beginPath();
      ctx.arc(ball.x, ball.y, ball.radius, 0, Math.PI * 2);
      ctx.fillStyle = "white";
      ctx.fill();
    }

    function drawHole() {
      ctx.beginPath();
      ctx.arc(hole.x, hole.y, hole.radius, 0, Math.PI * 2);
      ctx.fillStyle = "yellow"; 
      ctx.fill();
    }

    function drawObstacles() {
      obstacles.forEach(ob => {
        ctx.fillStyle = "red";
        ctx.fillRect(ob.x, ob.y, ob.w, ob.h);

        ob.y += 2 * ob.dir;
        if (ob.y <= 0 || ob.y + ob.h >= canvas.height) {
          ob.dir *= -1;
        }
      });
    }

    function checkCollision(ob) {
      return (
        ball.x + ball.radius > ob.x &&
        ball.x - ball.radius < ob.x + ob.w &&
        ball.y + ball.radius > ob.y &&
        ball.y - ball.radius < ob.y + ob.h
      );
    }

    function drawAimLine() {
      if (isDragging && mouseStart && mouseCurrent) {
        ctx.beginPath();
        ctx.setLineDash([5, 5]);
        ctx.moveTo(ball.x, ball.y);
        ctx.lineTo(mouseCurrent.x, mouseCurrent.y);
        ctx.strokeStyle = "lime";
        ctx.lineWidth = 2;
        ctx.stroke();
        ctx.setLineDash([]);
      }
    }
    function update() {
      if (isShooting) {
        
        ball.x += ball.dx;
        ball.y += ball.dy;

        ball.dx *= friction;
        ball.dy *= friction;

        if (ball.x <= ball.radius || ball.x >= canvas.width - ball.radius) ball.dx *= -1;
        if (ball.y <= ball.radius || ball.y >= canvas.height - ball.radius) ball.dy *= -1;

        obstacles.forEach(ob => {
          if (checkCollision(ob)) {
            ball.dx *= -1;
            ball.dy *= -1;
          }
        });

        const dist = Math.hypot(ball.x - hole.x, ball.y - hole.y);
        if (dist <= ball.radius + hole.radius) {
          alert("SCORE");
          sign.style.backgroundColor='#28a745';
          resetBall();
        }

        if (Math.abs(ball.dx) < 0.1 && Math.abs(ball.dy) < 0.1) {
          sign.style.backgroundColor='#28a745';
          isShooting = false;
        }
      }
    }

    function resetBall() {
      ball.x = initialBallPosition.x;
      ball.y = initialBallPosition.y;
      ball.dx = 0;
      ball.dy = 0;
      isShooting = false;
    }
    function drawforceangle(){

    }
    async function fetchData() {
        try{
            const response= await fetch('GET_status');
            data=await response.json();
            var content=`Force : ${data.force}\n Angle : ${data.angle} \n Trig : ${data.trig}`;
            iptForce.value=data.force;
            iptAngle.value=data.angle;
            online=true;
            cnctState.style.backgroundColor='#28a745';
            if(data.trig=='1'&&!isShooting ){
                theta=(iptAngle.value/180)*Math.PI;
                ball.dx=iptForce.value*Math.cos(theta);
                ball.dy=iptForce.value*Math.sin(theta);
                shotBtn.style.backgroundColor='orange';
                sign.style.backgroundColor='gray';
                isShooting=true;
            }else{
                shotBtn.style.backgroundColor='#28a745';
            }
            document.getElementById('content').innerText=content;
        }catch(error){
          online=false;
          cnctState.style.backgroundColor='red';
          console.error("Oops",error);
        }
    }
    function loop() {
      ctx.clearRect(0, 0, canvas.width, canvas.height);
      drawHole();
      drawObstacles();
      drawBall();
      drawAimLine();  
    ctx.font = "20px Arial";
    ctx.fillStyle = "white";
    ctx.fillText("FORCE: " + ((displaySpeed)/10).toFixed(2), 50, 50);
    ctx.fillText("ANGLE: " + displayAngle + "°", 50, 80);
    update();
      requestAnimationFrame(loop);
    }
    setInterval(fetchData,100);
    loop();

)===";
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  WiFi.config(localIp,dns,gateway,subnet);
  WiFi.begin(name,passw);
  pinMode(A0,INPUT);
  pinMode(8,INPUT);
  while(WiFi.status()!=WL_CONNECTED){
    Serial.print('.');
    delay(200);
  }
  Serial.println("connect");
  Serial.println(WiFi.localIP());
  server.begin();
}
String curline="";
String request[15]={"0"};
String jsonResponse ="";
int val,angle=0;
bool trig=0;
int i=0;
void fetchRTrespond(WiFiClient client){
  val=map(analogRead(A0),0,1023,0,100);
  trig=digitalRead(8);
  //angle=
  jsonResponse="{\"force\":\""+String(val)+ "\",\n \"angle\":\""+String(angle)+"\",\n \"trig\":\""+String(trig)+ "\"}";        //{"Force":"${val}", \n "Angle":"${angle}", \n "trig":"${istrig}"}
  //Serial.println(jsonResponse);
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Access-Control-Allow-Methods: GET, POST, OPTIONS");
  client.println("Access-Control-Allow-Headers: Content-Type");
  client.println("Connection: close");
  client.println("Content-Length: " + String(jsonResponse.length()));
  client.println();
  client.println(jsonResponse);
}
void htmlPage(WiFiClient client){
  
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Access-Control-Allow-Methods: GET, POST, OPTIONS");
  client.println("Access-Control-Allow-Headers: Content-Type");
  client.println("Connection: close");
  client.println("Content-Length: " + html.length());
  client.println();
  client.println(html); 
}
void cssPage(WiFiClient client){
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/css");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Access-Control-Allow-Methods: GET, POST, OPTIONS");
  client.println("Access-Control-Allow-Headers: Content-Type");
  client.println("Connection: close");
  client.println("Content-Length: " + css.length());
  client.println();
  client.println(css); 
}
void JSContent(WiFiClient client){
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/javascript");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Access-Control-Allow-Methods: GET, POST, OPTIONS");
  client.println("Access-Control-Allow-Headers: Content-Type");
  client.println("Connection: close");
  client.println("Content-Length: " + js.length());
  client.println();
  client.println(js); 
}
void loop() {
  WiFiClient client=server.available();
  if(client){
    
    Serial.println("client connect");
    
    while(client.connected()){
      if(client.available()){
        char c=client.read();
        //Serial.write(c);
        if(c=='\n'){
          if(curline.length()==0){    //request has ended
            Serial.println(request[0]);
            if(request[0].indexOf("GET / ")>-1){
              htmlPage(client);
            }else if(request[0].indexOf("GET /style.css")>-1){
              cssPage(client);
            }else if(request[0].indexOf("GET /script.js")>-1){
              JSContent(client);
            }else if(request[0].indexOf("/GET_status")>-1){
              fetchRTrespond(client);
            }
            break;
          }else{
            request[i]=curline;
            curline="";
            i++;
          }
        }else if(c!='\r'){
          curline+=c;
        }
      }
    }
    client.stop();
  }
  i=0;
  /*for(int j=0;j<15;j++){
    request[j]="";
  }*/
  //delay(1);
  // put your main code here, to run repeatedly:
  
  
}
