#pragma once
#include <Arduino.h>

// Offline Dark-Mode Web Portal stored in Flash (PROGMEM)
// Zero external CDN dependencies - 100% offline captive portal
static const char PORTAL_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32-S3 WPS Credentials Portal</title>
  <style>
    :root {
      --bg: #0f172a;
      --card-bg: #1e293b;
      --card-border: #334155;
      --text-main: #f8fafc;
      --text-muted: #94a3b8;
      --accent: #38bdf8;
      --accent-hover: #0284c7;
      --success: #10b981;
      --success-bg: rgba(16, 185, 129, 0.15);
      --font-mono: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, "Liberation Mono", "Courier New", monospace;
      --font-sans: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      background-color: var(--bg);
      color: var(--text-main);
      font-family: var(--font-sans);
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      padding: 1.25rem;
    }
    .container {
      width: 100%;
      max-width: 460px;
      background: var(--card-bg);
      border: 1px solid var(--card-border);
      border-radius: 1rem;
      padding: 1.75rem;
      box-shadow: 0 20px 25px -5px rgba(0, 0, 0, 0.5), 0 8px 10px -6px rgba(0, 0, 0, 0.5);
    }
    .header {
      display: flex;
      align-items: center;
      gap: 0.75rem;
      margin-bottom: 1.25rem;
    }
    .logo-badge {
      width: 42px;
      height: 42px;
      border-radius: 0.625rem;
      background: linear-gradient(135deg, #0284c7, #38bdf8);
      display: flex;
      align-items: center;
      justify-content: center;
      box-shadow: 0 4px 12px rgba(56, 189, 248, 0.3);
    }
    .logo-badge svg {
      width: 24px;
      height: 24px;
      fill: #ffffff;
    }
    .header-text h1 {
      font-size: 1.25rem;
      font-weight: 700;
      color: var(--text-main);
      letter-spacing: -0.025em;
    }
    .header-text p {
      font-size: 0.8125rem;
      color: var(--text-muted);
    }
    .badge {
      display: inline-flex;
      align-items: center;
      gap: 0.5rem;
      background: var(--success-bg);
      border: 1px solid rgba(16, 185, 129, 0.3);
      color: var(--success);
      padding: 0.375rem 0.75rem;
      border-radius: 9999px;
      font-size: 0.75rem;
      font-weight: 600;
      margin-bottom: 1.5rem;
    }
    .badge-dot {
      width: 7px;
      height: 7px;
      background-color: var(--success);
      border-radius: 50%;
      box-shadow: 0 0 8px var(--success);
      animation: pulse 2s infinite;
    }
    @keyframes pulse {
      0%, 100% { opacity: 1; transform: scale(1); }
      50% { opacity: 0.4; transform: scale(0.85); }
    }
    .field-group {
      margin-bottom: 1.25rem;
    }
    .field-label {
      font-size: 0.75rem;
      text-transform: uppercase;
      letter-spacing: 0.05em;
      color: var(--text-muted);
      margin-bottom: 0.375rem;
      display: flex;
      justify-content: space-between;
    }
    .input-box {
      display: flex;
      align-items: center;
      background: #090d16;
      border: 1px solid var(--card-border);
      border-radius: 0.625rem;
      padding: 0.625rem 0.875rem;
      gap: 0.5rem;
      transition: border-color 0.15s;
    }
    .input-box:focus-within {
      border-color: var(--accent);
    }
    .input-val {
      font-family: var(--font-mono);
      font-size: 0.9375rem;
      color: var(--text-main);
      flex: 1;
      overflow: hidden;
      text-overflow: ellipsis;
      white-space: nowrap;
      user-select: all;
    }
    .btn-icon {
      background: transparent;
      border: none;
      cursor: pointer;
      color: var(--text-muted);
      padding: 0.25rem;
      border-radius: 0.375rem;
      display: flex;
      align-items: center;
      justify-content: center;
      transition: color 0.15s, background-color 0.15s;
    }
    .btn-icon:hover {
      color: var(--accent);
      background-color: rgba(56, 189, 248, 0.1);
    }
    .btn-icon svg {
      width: 18px;
      height: 18px;
      stroke: currentColor;
      fill: none;
      stroke-width: 2;
      stroke-linecap: round;
      stroke-linejoin: round;
    }
    .btn-copy {
      background: #1e293b;
      border: 1px solid var(--card-border);
      color: var(--text-main);
      padding: 0.35rem 0.65rem;
      border-radius: 0.375rem;
      font-size: 0.75rem;
      font-weight: 500;
      cursor: pointer;
      transition: all 0.15s;
      white-space: nowrap;
    }
    .btn-copy:hover {
      border-color: var(--accent);
      color: var(--accent);
    }
    .btn-copy.copied {
      background: var(--success);
      border-color: var(--success);
      color: #ffffff;
    }
    .qr-card {
      background: #ffffff;
      border-radius: 0.875rem;
      padding: 1.25rem;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      margin-top: 1.25rem;
      box-shadow: 0 4px 12px rgba(0, 0, 0, 0.2);
    }
    #qr-canvas {
      width: 180px;
      height: 180px;
      image-rendering: pixelated;
    }
    .qr-hint {
      color: #475569;
      font-size: 0.75rem;
      font-weight: 500;
      margin-top: 0.75rem;
      text-align: center;
    }
    .info-grid {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 0.75rem;
      margin-top: 1.25rem;
      padding-top: 1.25rem;
      border-top: 1px solid var(--card-border);
    }
    .info-item {
      background: #090d16;
      padding: 0.625rem 0.75rem;
      border-radius: 0.5rem;
      border: 1px solid rgba(51, 65, 85, 0.5);
    }
    .info-label {
      font-size: 0.6875rem;
      color: var(--text-muted);
      text-transform: uppercase;
      letter-spacing: 0.05em;
    }
    .info-value {
      font-family: var(--font-mono);
      font-size: 0.8125rem;
      color: var(--text-main);
      margin-top: 0.25rem;
    }
    .footer {
      margin-top: 1.5rem;
      display: flex;
      justify-content: space-between;
      align-items: center;
      font-size: 0.75rem;
      color: var(--text-muted);
    }
    .api-link {
      color: var(--accent);
      text-decoration: none;
      font-family: var(--font-mono);
      font-size: 0.75rem;
    }
    .api-link:hover {
      text-decoration: underline;
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <div class="logo-badge">
        <svg viewBox="0 0 24 24"><path d="M12 18a2 2 0 1 0 0-4 2 2 0 0 0 0 4zm7.07-7.07a10 10 0 0 0-14.14 0l1.41 1.41a8 8 0 0 1 11.32 0l1.41-1.41zm3.54-3.54a15 15 0 0 0-21.22 0l1.42 1.42a13 13 0 0 1 18.38 0l1.42-1.42z"/></svg>
      </div>
      <div class="header-text">
        <h1>WPS Credential Portal</h1>
        <p>ESP32-S3 Standalone Fetcher</p>
      </div>
    </div>

    <div class="badge">
      <span class="badge-dot"></span>
      <span>Wi-Fi Credentials Captured via WPS PBC</span>
    </div>

    <!-- SSID Field -->
    <div class="field-group">
      <div class="field-label">
        <span>Network Name (SSID)</span>
      </div>
      <div class="input-box">
        <span class="input-val" id="ssid-val">%SSID%</span>
        <button class="btn-copy" onclick="copyText('ssid-val', this)">Copy</button>
      </div>
    </div>

    <!-- Password Field -->
    <div class="field-group">
      <div class="field-label">
        <span>Wi-Fi Password</span>
      </div>
      <div class="input-box">
        <span class="input-val" id="pwd-val" data-raw="%PASSWORD%">%MASKED_PASSWORD%</span>
        <button class="btn-icon" id="toggle-eye" onclick="togglePassword()" title="Show/Hide Password">
          <svg id="eye-icon" viewBox="0 0 24 24">
            <path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"></path>
            <circle cx="12" cy="12" r="3"></circle>
          </svg>
        </button>
        <button class="btn-copy" onclick="copyRawPassword(this)">Copy</button>
      </div>
    </div>

    <!-- QR Code Scan -->
    <div class="qr-card">
      <canvas id="qr-canvas"></canvas>
      <div class="qr-hint">Scan with your phone camera to connect automatically</div>
    </div>

    <!-- System Diagnostics -->
    <div class="info-grid">
      <div class="info-item">
        <div class="info-label">SoftAP Gateway</div>
        <div class="info-value">192.168.4.1</div>
      </div>
      <div class="info-item">
        <div class="info-label">Security Type</div>
        <div class="info-value">WPA / WPA2-PSK</div>
      </div>
      <div class="info-item">
        <div class="info-label">Hardware Target</div>
        <div class="info-value">ESP32-S3 (Standalone)</div>
      </div>
      <div class="info-item">
        <div class="info-label">Status LED</div>
        <div class="info-value">GPIO 48 WS2812</div>
      </div>
    </div>

    <div class="footer">
      <span>Portal: ESP32-WPS-Portal</span>
      <a class="api-link" href="/api/credentials" target="_blank">JSON API &rarr;</a>
    </div>
  </div>

  <script>
    // Pure Vanilla Offline QR Code Generator (Kazuhiko Arase QR Model)
    // Supports ISO/IEC 18004 QR Codes without any external library or network access
    (function(){
      var QRMode={MODE_NUMBER:1,MODE_ALPHA_NUM:2,MODE_8BIT_BYTE:4,MODE_KANJI:8};
      var QRErrorCorrectLevel={L:1,M:0,Q:3,H:2};
      var QRMaskPattern={PATTERN000:0,PATTERN001:1,PATTERN010:2,PATTERN011:3,PATTERN100:4,PATTERN101:5,PATTERN110:6,PATTERN111:7};
      var QRUtil={
        PATTERN_POSITION_TABLE:[[],[6,18],[6,22],[6,26],[6,30],[6,34],[6,22,38],[6,24,42],[6,26,46],[6,28,50],[6,30,54]],
        G15:(1<<10)|(1<<8)|(1<<5)|(1<<4)|(1<<2)|(1<<1)|(1<<0),
        G18:(1<<12)|(1<<11)|(1<<10)|(1<<9)|(1<<8)|(1<<5)|(1<<2)|(1<<0),
        G15_MASK:(1<<14)|(1<<12)|(1<<10)|(1<<4)|(1<<1),
        getBCHTypeInfo:function(d){var d5=d<<10;while(QRUtil.getBCHDigit(d5)-QRUtil.getBCHDigit(QRUtil.G15)>=0){d5^=(QRUtil.G15<<(QRUtil.getBCHDigit(d5)-QRUtil.getBCHDigit(QRUtil.G15)));}return((d<<10)|d5)^QRUtil.G15_MASK;},
        getBCHDigit:function(d){var a=0;while(d!=0){a++;d>>>=1;}return a;},
        getPatternPosition:function(typeNumber){return QRUtil.PATTERN_POSITION_TABLE[typeNumber-1];},
        getMask:function(maskPattern,i,j){switch(maskPattern){case 0:return(i+j)%2==0;case 1:return i%2==0;case 2:return j%3==0;case 3:return(i+j)%3==0;case 4:return(Math.floor(i/2)+Math.floor(j/3))%2==0;case 5:return(i*j)%2+(i*j)%3==0;case 6:return((i*j)%2+(i*j)%3)%2==0;case 7:return((i*j)%3+(i+j)%2)%2==0;default:throw new Error("bad mask:"+maskPattern);}},
        getErrorCorrectPolynomial:function(errorCorrectLength){var a=new QRPolynomial([1],0);for(var i=0;i<errorCorrectLength;i++){a=a.multiply(new QRPolynomial([1,QRMath.gexp(i)],0));}return a;},
        getLengthInBits:function(mode,type){if(1<=type&&type<10){switch(mode){case 1:return 10;case 2:return 9;case 4:return 8;case 8:return 8;default:throw new Error("m:"+mode);}}else{return 16;}}
      };
      var QRMath={glog:function(n){if(n<1){throw new Error("glog("+n+")");}return QRMath.LOG_TABLE[n];},gexp:function(n){while(n<0){n+=255;}while(n>=255){n-=255;}return QRMath.EXP_TABLE[n];},EXP_TABLE:new Array(256),LOG_TABLE:new Array(256)};
      for(var i=0;i<8;i++){QRMath.EXP_TABLE[i]=1<<i;}
      for(var i=8;i<256;i++){QRMath.EXP_TABLE[i]=QRMath.EXP_TABLE[i-4]^QRMath.EXP_TABLE[i-5]^QRMath.EXP_TABLE[i-6]^QRMath.EXP_TABLE[i-8];}
      for(var i=0;i<255;i++){QRMath.LOG_TABLE[QRMath.EXP_TABLE[i]]=i;}
      function QRPolynomial(num,shift){if(num.length==undefined){throw new Error(num.length+"/"+shift);}var offset=0;while(offset<num.length&&num[offset]==0){offset++;}this.num=new Array(num.length-offset+shift);for(var i=0;i<num.length-offset;i++){this.num[i]=num[i+offset];}}
      QRPolynomial.prototype={get:function(index){return this.num[index];},getLength:function(){return this.num.length;},multiply:function(e){var num=new Array(this.getLength()+e.getLength()-1);for(var i=0;i<this.getLength();i++){for(var j=0;j<e.getLength();j++){num[i+j]^=QRMath.gexp(QRMath.glog(this.get(i))+QRMath.glog(e.get(j)));}}return new QRPolynomial(num,0);},mod:function(e){if(this.getLength()-e.getLength()<0){return this;}var ratio=QRMath.glog(this.get(0))-QRMath.glog(e.get(0));var num=new Array(this.getLength());for(var i=0;i<this.getLength();i++){num[i]=this.get(i);}for(var i=0;i<e.getLength();i++){num[i]^=QRMath.gexp(QRMath.glog(e.get(i))+ratio);}return new QRPolynomial(num,0).mod(e);}};
      var QRRSBlock={
        RS_BLOCK_TABLE:[
          [1,26,19],[1,26,16],[1,26,13],[1,26,9],
          [1,44,34],[1,44,28],[1,44,22],[1,44,16],
          [1,70,55],[1,70,44],[2,35,17],[2,35,13],
          [1,100,80],[2,50,32],[2,50,24],[4,25,9],
          [1,134,108],[2,67,43],[2,33,15,2,34,16],[2,33,11,2,34,12],
          [2,86,68],[4,43,27],[4,43,19],[4,43,15],
          [2,98,78],[4,49,31],[2,32,14,4,33,15],[4,39,13,1,40,14],
          [2,121,97],[2,60,38,2,61,39],[4,40,18,2,41,19],[4,40,14,2,41,15],
          [2,146,116],[3,58,36,2,59,37],[4,36,16,4,37,17],[4,36,12,4,37,13],
          [2,86,68,2,87,69],[4,69,43,1,70,44],[6,43,19,2,44,20],[6,43,15,2,44,16]
        ],
        getRSBlocks:function(typeNumber,errorCorrectLevel){
          var rsBlock=QRRSBlock.getRsBlockTable(typeNumber,errorCorrectLevel);
          if(rsBlock==undefined)throw new Error("bad rs block");
          var length=rsBlock.length/3;var list=[];
          for(var i=0;i<length;i++){var count=rsBlock[i*3+0];var totalCount=rsBlock[i*3+1];var dataCount=rsBlock[i*3+2];for(var j=0;j<count;j++){list.push({totalCount:totalCount,dataCount:dataCount});}}
          return list;
        },
        getRsBlockTable:function(typeNumber,errorCorrectLevel){switch(errorCorrectLevel){case 1:return QRRSBlock.RS_BLOCK_TABLE[(typeNumber-1)*4+0];case 0:return QRRSBlock.RS_BLOCK_TABLE[(typeNumber-1)*4+1];case 3:return QRRSBlock.RS_BLOCK_TABLE[(typeNumber-1)*4+2];case 2:return QRRSBlock.RS_BLOCK_TABLE[(typeNumber-1)*4+3];}}
      };
      function QRBitBuffer(){this.buffer=[];this.length=0;}
      QRBitBuffer.prototype={get:function(index){var bufIndex=Math.floor(index/8);return((this.buffer[bufIndex]>>>(7-index%8))&1)==1;},put:function(num,length){for(var i=0;i<length;i++){this.putBit(((num>>>(length-i-1))&1)==1);}},putBit:function(bit){var bufIndex=Math.floor(this.length/8);if(this.buffer.length<=bufIndex){this.buffer.push(0);}if(bit){this.buffer[bufIndex]|=(0x80>>>(this.length%8));}this.length++;}};
      function QR8bitByte(data){this.mode=4;this.data=data;}
      QR8bitByte.prototype={getLength:function(){return this.data.length;},write:function(buffer){for(var i=0;i<this.data.length;i++){buffer.put(this.data.charCodeAt(i),8);}}};
      function QRCodeModel(typeNumber,errorCorrectLevel){this.typeNumber=typeNumber;this.errorCorrectLevel=errorCorrectLevel;this.modules=null;this.moduleCount=0;this.dataCache=null;this.dataList=[];}
      QRCodeModel.prototype={
        addData:function(data){this.dataList.push(new QR8bitByte(data));this.dataCache=null;},
        isDark:function(row,col){return this.modules[row][col];},
        getModuleCount:function(){return this.moduleCount;},
        make:function(){
          if(this.typeNumber<1){
            for(var t=1;t<=10;t++){
              var rs=QRRSBlock.getRSBlocks(t,this.errorCorrectLevel);
              var totalDataCount=0;
              for(var k=0;k<rs.length;k++){totalDataCount+=rs[k].dataCount;}
              var byteLen=0;
              for(var d=0;d<this.dataList.length;d++){byteLen+=this.dataList[d].getLength();}
              if(byteLen+3<=totalDataCount){this.typeNumber=t;break;}
            }
          }
          this.makeImpl(false,this.getBestMaskPattern());
        },
        makeImpl:function(test,maskPattern){
          this.moduleCount=this.typeNumber*4+17;
          this.modules=new Array(this.moduleCount);
          for(var row=0;row<this.moduleCount;row++){this.modules[row]=new Array(this.moduleCount);for(var col=0;col<this.moduleCount;col++){this.modules[row][col]=null;}}
          this.setupPositionProbePattern(0,0);this.setupPositionProbePattern(this.moduleCount-7,0);this.setupPositionProbePattern(0,this.moduleCount-7);
          this.setupPositionAdjustPattern();this.setupTimingPattern();this.setupTypeInfo(test,maskPattern);
          if(this.dataCache==null){this.dataCache=QRCodeModel.createData(this.typeNumber,this.errorCorrectLevel,this.dataList);}
          this.mapData(this.dataCache,maskPattern);
        },
        setupPositionProbePattern:function(row,col){for(var r=-1;r<=7;r++){if(row+r<=-1||this.moduleCount<=row+r)continue;for(var c=-1;c<=7;c++){if(col+c<=-1||this.moduleCount<=col+c)continue;if((0<=r&&r<=6&&(c==0||c==6))||(0<=c&&c<=6&&(r==0||r==6))||(2<=r&&r<=4&&2<=c&&c<=4)){this.modules[row+r][col+c]=true;}else{this.modules[row+r][col+c]=false;}}}},
        getBestMaskPattern:function(){var minLostPoint=0;var pattern=0;for(var i=0;i<8;i++){this.makeImpl(true,i);var lostPoint=this.getLostPoint();if(i==0||minLostPoint>lostPoint){minLostPoint=lostPoint;pattern=i;}}return pattern;},
        setupTimingPattern:function(){for(var r=8;r<this.moduleCount-8;r++){if(this.modules[r][6]!=null)continue;this.modules[r][6]=(r%2==0);}for(var c=8;c<this.moduleCount-8;c++){if(this.modules[6][c]!=null)continue;this.modules[6][c]=(c%2==0);}},
        setupPositionAdjustPattern:function(){var pos=QRUtil.getPatternPosition(this.typeNumber);for(var i=0;i<pos.length;i++){for(var j=0;j<pos.length;j++){var row=pos[i];var col=pos[j];if(this.modules[row][col]!=null)continue;for(var r=-2;r<=2;r++){for(var c=-2;c<=2;c++){if(r==-2||r==2||c==-2||c==2||(r==0&&c==0)){this.modules[row+r][col+c]=true;}else{this.modules[row+r][col+c]=false;}}}}}},
        setupTypeInfo:function(test,maskPattern){var data=(this.errorCorrectLevel<<3)|maskPattern;var bits=QRUtil.getBCHTypeInfo(data);for(var i=0;i<15;i++){var mod=(!test&&((bits>>i)&1)==1);if(i<6){this.modules[i][8]=mod;}else if(i<8){this.modules[i+1][8]=mod;}else{this.modules[this.moduleCount-15+i][8]=mod;}if(i<8){this.modules[8][this.moduleCount-i-1]=mod;}else if(i<9){this.modules[8][15-i-1+1]=mod;}else{this.modules[8][15-i-1]=mod;}}this.modules[this.moduleCount-8][8]=(!test);},
        mapData:function(data,maskPattern){var inc=-1;var row=this.moduleCount-1;var bitIndex=7;var byteIndex=0;for(var col=this.moduleCount-1;col>0;col-=2){if(col==6)col--;while(true){for(var c=0;c<2;c++){if(this.modules[row][col-c]==null){var dark=false;if(byteIndex<data.length){dark=(((data[byteIndex]>>>bitIndex)&1)==1);}var mask=QRUtil.getMask(maskPattern,row,col-c);if(mask){dark=!dark;}this.modules[row][col-c]=dark;bitIndex--;if(bitIndex==-1){byteIndex++;bitIndex=7;}}}row+=inc;if(row<0||this.moduleCount<=row){row-=inc;inc=-inc;break;}}}},
        getLostPoint:function(){var moduleCount=this.moduleCount;var lostPoint=0;for(var row=0;row<moduleCount;row++){for(var col=0;col<moduleCount;col++){var sameCount=0;var dark=this.modules[row][col];for(var r=-1;r<=1;r++){if(row+r<0||moduleCount<=row+r)continue;for(var c=-1;c<=1;c++){if(col+c<0||moduleCount<=col+c)continue;if(r==0&&c==0)continue;if(dark==this.modules[row+r][col+c]){sameCount++;}}}if(sameCount>5){lostPoint+=(3+sameCount-5);}}}return lostPoint;}
      };
      QRCodeModel.createData=function(typeNumber,errorCorrectLevel,dataList){
        var rsBlocks=QRRSBlock.getRSBlocks(typeNumber,errorCorrectLevel);
        var buffer=new QRBitBuffer();
        for(var i=0;i<dataList.length;i++){var data=dataList[i];buffer.put(data.mode,4);buffer.put(data.getLength(),QRUtil.getLengthInBits(data.mode,typeNumber));data.write(buffer);}
        var totalDataCount=0;for(var i=0;i<rsBlocks.length;i++){totalDataCount+=rsBlocks[i].dataCount;}
        if(buffer.length+4<=totalDataCount*8){buffer.put(0,4);}
        while(buffer.length%8!=0){buffer.putBit(false);}
        while(true){if(buffer.length>=totalDataCount*8){break;}buffer.put(0xEC,8);if(buffer.length>=totalDataCount*8){break;}buffer.put(0x11,8);}
        return QRCodeModel.createBytes(buffer,rsBlocks);
      };
      QRCodeModel.createBytes=function(buffer,rsBlocks){
        var offset=0;var maxDcCount=0;var maxEcCount=0;
        var dcdata=new Array(rsBlocks.length);var ecdata=new Array(rsBlocks.length);
        for(var r=0;r<rsBlocks.length;r++){
          var dcCount=rsBlocks[r].dataCount;var ecCount=rsBlocks[r].totalCount-dcCount;
          maxDcCount=Math.max(maxDcCount,dcCount);maxEcCount=Math.max(maxEcCount,ecCount);
          dcdata[r]=new Array(dcCount);for(var i=0;i<dcdata[r].length;i++){dcdata[r][i]=0xFF&buffer.buffer[i+offset];}offset+=dcCount;
          var rsPoly=QRUtil.getErrorCorrectPolynomial(ecCount);
          var rawPoly=new QRPolynomial(dcdata[r],rsPoly.getLength()-1);
          var modPoly=rawPoly.mod(rsPoly);
          ecdata[r]=new Array(rsPoly.getLength()-1);
          for(var i=0;i<ecdata[r].length;i++){var modIndex=i+modPoly.getLength()-ecdata[r].length;ecdata[r][i]=(modIndex>=0)?modPoly.get(modIndex):0;}
        }
        var totalCodeCount=0;for(var i=0;i<rsBlocks.length;i++){totalCodeCount+=rsBlocks[i].totalCount;}
        var data=new Array(totalCodeCount);var index=0;
        for(var i=0;i<maxDcCount;i++){for(var r=0;r<rsBlocks.length;r++){if(i<dcdata[r].length){data[index++]=dcdata[r][i];}}}
        for(var i=0;i<maxEcCount;i++){for(var r=0;r<rsBlocks.length;r++){if(i<ecdata[r].length){data[index++]=ecdata[r][i];}}}
        return data;
      };
      window.QRCode={
        render:function(canvas,text){
          var qr=new QRCodeModel(0,QRErrorCorrectLevel.M);
          qr.addData(text);
          qr.make();
          var count=qr.getModuleCount();
          var size=180;
          var scale=Math.floor(size/count);
          if(scale<1)scale=1;
          var margin=Math.floor((size-(count*scale))/2);
          canvas.width=size;
          canvas.height=size;
          var ctx=canvas.getContext('2d');
          ctx.fillStyle='#ffffff';
          ctx.fillRect(0,0,size,size);
          ctx.fillStyle='#0f172a';
          for(var r=0;r<count;r++){
            for(var c=0;c<count;c++){
              if(qr.isDark(r,c)){
                ctx.fillRect(margin+c*scale,margin+r*scale,scale,scale);
              }
            }
          }
        }
      };
    })();

    // Password show / hide toggle
    var isPasswordVisible = false;
    function togglePassword() {
      var pwdSpan = document.getElementById('pwd-val');
      var rawPwd = pwdSpan.getAttribute('data-raw');
      var eyeIcon = document.getElementById('eye-icon');
      isPasswordVisible = !isPasswordVisible;
      if (isPasswordVisible) {
        pwdSpan.innerText = rawPwd;
        eyeIcon.innerHTML = '<path d="M17.94 17.94A10.07 10.07 0 0 1 12 20c-7 0-11-8-11-8a18.45 18.45 0 0 1 5.06-5.94M9.9 4.24A9.12 9.12 0 0 1 12 4c7 0 11 8 11 8a18.5 18.5 0 0 1-2.16 3.19m-6.72-1.07a3 3 0 1 1-4.24-4.24"></path><line x1="1" y1="1" x2="23" y2="23"></line>';
      } else {
        pwdSpan.innerText = "••••••••";
        eyeIcon.innerHTML = '<path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"></path><circle cx="12" cy="12" r="3"></circle>';
      }
    }

    // Copy handlers with visual feedback
    function copyText(elemId, btn) {
      var text = document.getElementById(elemId).innerText.trim();
      navigator.clipboard.writeText(text).then(function() {
        btn.innerText = 'Copied!';
        btn.classList.add('copied');
        setTimeout(function() {
          btn.innerText = 'Copy';
          btn.classList.remove('copied');
        }, 2000);
      });
    }

    function copyRawPassword(btn) {
      var rawPwd = document.getElementById('pwd-val').getAttribute('data-raw');
      navigator.clipboard.writeText(rawPwd).then(function() {
        btn.innerText = 'Copied!';
        btn.classList.add('copied');
        setTimeout(function() {
          btn.innerText = 'Copy';
          btn.classList.remove('copied');
        }, 2000);
      });
    }

    // Auto-generate Wi-Fi QR Code on page load
    window.onload = function() {
      var ssid = "%SSID%";
      var password = "%PASSWORD%";
      // Standard Wi-Fi connection QR payload: WIFI:T:WPA;S:<SSID>;P:<PASSWORD>;;
      var wifiQrPayload = "WIFI:T:WPA;S:" + ssid + ";P:" + password + ";;";
      try {
        var canvas = document.getElementById('qr-canvas');
        window.QRCode.render(canvas, wifiQrPayload);
      } catch (err) {
        console.error("QR Code generation error:", err);
      }
    };
  </script>
</body>
</html>
)rawliteral";
