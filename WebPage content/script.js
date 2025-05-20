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
      radius: 25,
      rds_decreaser:0.8, //min->10
      limitSpeed: 10,  //最高進洞容許
      MinSpeed:3,    //至此不再降低
      spd_decreaser:0.9,
    };
    let map = {
      0:{color:'blue',fric:0.988,scene:'ice'},
      1:{color:'brown',fric:0.96,scene:'wood'},
      2:{color : 'green',fric:0.92,scene:'grass'},
      3:{color:'gray',fric:0.88,scene:'concrete'},
    };
    
    const obstacles = [
      { x: 800, y: 100, w: 10, h: 100, dir: 1 },
      { x: 500, y: 300, w: 10, h: 100, dir: -1 },
    ];
    let totalpoint=0,angle=0,force=0
    let friction = 0.988;
    let displaySpeed,displayAngle=0;
    let isShooting,isDragging,online = false;
    let mouseStart,mouseCurrent = null;
    var shotBtn= document.getElementById('istrig');
    var iptForce=document.getElementById('force');
    var iptAngle=document.getElementById('angle');
    var sign=document.getElementById('sign');
    var cnctState=document.getElementById('cnct_state');
    var timer=document.getElementById('timer');
    var scnlabel=document.getElementById('scene');
    var theta=0;
    shotBtn.addEventListener('click',()=>{
      angle=iptAngle.value;
      if(isShooting==false){
        theta=(angle/180)*Math.PI;
        ball.dx=force*Math.cos(theta);
        ball.dy=force*Math.sin(theta);
        sign.style.backgroundColor='gray';
        isShooting=true;
      }
    });
    iptForce.addEventListener('change',()=>{sign.style.backgroundColor='yellow'; force=iptForce.value;});
    iptAngle.addEventListener('change',()=>{sign.style.backgroundColor='yellow'; angle=iptAngle.valu;});
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

        const centerX = canvas.width / 2;
        const centerY = canvas.height / 2;
        const dxCenter = e.offsetX - centerX;
        const dyCenter = e.offsetY - centerY;
        angle = Math.atan2(dyCenter, dxCenter);  // 滑鼠控制角度
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

function drawDegreeIndicator(x, y, r, degree, color = "cyan") {
  // 把度數轉成弧度，並轉換成Canvas的座標系（0度在右，逆時針增加）
  const rad = (degree) * Math.PI / 180;

  // 呼叫原本的畫指針函式
  drawAngleIndicator(x, y, r, rad, color);
}

function drawAngleIndicator(x, y, r, angle, color = "red") {
  // 外圓
  ctx.beginPath();
  ctx.arc(x, y, r, 0, Math.PI * 2);
  ctx.strokeStyle = "black";
  ctx.lineWidth = 2;
  ctx.stroke();

  // 指針
  const lineLength = r - 5;
  const endX = x + Math.cos(angle) * lineLength;
  const endY = y + Math.sin(angle) * lineLength;

  ctx.beginPath();
  ctx.moveTo(x, y);
  ctx.lineTo(endX, endY);
  ctx.strokeStyle = color;
  ctx.lineWidth = 3;
  ctx.stroke();
}


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
    
   function resetHole() {
      const margin = 10;
      const holeRadius = hole.radius;
      const maxAttempts = 100;

      for (let attempt = 0; attempt < maxAttempts; attempt++) {
        const newX = Math.floor(Math.random() * (canvas.width / 2 - 2 * holeRadius - margin)) + canvas.width / 2 + holeRadius + margin;
        const newY = Math.floor(Math.random() * (canvas.height - 2 * holeRadius - 2 * margin)) + holeRadius + margin;

        let isSafe = true;
        for (let ob of obstacles) {
          const obLeft = ob.x - margin;
          const obRight = ob.x + ob.w + margin;
          const obTop = ob.y - margin;
          const obBottom = ob.y + ob.h + margin;

          if (
            newX + holeRadius > obLeft &&
            newX - holeRadius < obRight &&
            newY + holeRadius > obTop &&
            newY - holeRadius < obBottom
          ) {
            isSafe = false;
            break;
          }
        }

        if (isSafe) {
          hole.x = newX;
          hole.y = newY;
          return;
        }
      }
      console.warn("找不到安全的洞位置！");
    }
    function setMap(index){
      canvas.style.backgroundColor=map[index].color;
      friction=map[index].fric;
      scnlabel.innerHTML=map[index].scene;
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
        if ((dist <= ball.radius + hole.radius)&&(Math.sqrt(ball.dx*ball.dx+ball.dy+ball.dy)<hole.limitSpeed)) {
          alert("SCORE");
          try{
            setMap(Math.floor(Math.random()*4));
          }catch(e){
            console.error(e);
          }
          
          totalpoint+=1
          hole.limitSpeed=(hole.limitSpeed*hole.spd_decreaser<hole.MinSpeed)? hole.MinSpeed:hole.limitSpeed*hole.spd_decreaser;
          hole.radius=(hole.radius<=5)? 5:hole.radius*hole.rds_decreaser; 
          sign.style.backgroundColor='#28a745';
          resetBall();
          resetHole();
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
    function drawAngleIndicator(x, y, r, angle,color = "red") {
        ctx.beginPath();
        ctx.arc(x, y, r, 0, Math.PI * 2);
        ctx.strokeStyle = "black";
        ctx.lineWidth = 2;
        ctx.stroke();

        const lineLength = r - 5;
        const endX = x + Math.cos(angle) * lineLength;
        const endY = y + Math.sin(angle) * lineLength;

        ctx.beginPath();
        ctx.moveTo(x, y);
        ctx.lineTo(endX, endY);
        ctx.strokeStyle = color;
        ctx.lineWidth = 3;
        ctx.stroke();
      }
    
    async function fetchData() {
        try{
            const response= await fetch('GET_status');
            data=await response.json();
            iptForce.value=force=data.force;
            iptAngle.value=angle=data.angle;
            drawDegreeIndicator(canvas.width - 60, 60, 30, Number(angle), "cyan");
            online=true;
            cnctState.style.backgroundColor='#28a745';
            if(data.trig=='1'&&!isShooting ){
                theta=(iptAngle.value/180)*Math.PI;
                ball.dx=force*Math.cos(theta);
                ball.dy=force*Math.sin(theta);
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
    ctx.fillText("SPEED : " + (displaySpeed/10).toFixed(2), 50, 50);
    ctx.fillText("LIMIT< " + hole.limitSpeed.toFixed(2) , 50, 80);
    ctx.fillText("SCORES :"+(totalpoint),50,110)
      //drawAngleIndicator(canvas.width - 60, 60, 30, angleFromArduino, "red");
     // drawAngleIndicator(canvas.width - 60, 60, 30, angle, "yellow");
     drawDegreeIndicator(canvas.width - 60, 60, 30, Number(iptAngle.value), "cyan");
    update();
      requestAnimationFrame(loop);
    }
    
    setInterval(fetchData,100);
    loop();
