

#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WebServer.h>
#include <ESP32Servo.h>

// ---------------- LCD ----------------
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------------- Pins ----------------
#define IR_ENTRY 18
#define IR_EXIT 19
#define SERVO_PIN 13
#define GREEN_LED 26
#define RED_LED 27
#define BUZZER 14
// ---------------- Servo ----------------
Servo barrierServo;

// Change these lines in your sketch:
const char* ssid = "prathamesh17";
const char* password = "vvvpsh77";
// ---------------- Web Server ----------------
WebServer server(80);

// ---------------- Parking Variables ----------------
int vehiclesPassed = 0;
int tollCollected = 0;
const int tollPerVehicle = 50;

bool entryTriggered = false;
bool exitTriggered = false;

String gateStatus = "Closed";

// ---------------- Timing ----------------
unsigned long gateTimer = 0;
bool gateOpen = false;

// ---------------- LCD Update ----------------
void updateLCD() {

  lcd.setCursor(0, 0);
  lcd.print("Vehicles:");
  lcd.print(vehiclesPassed);
  lcd.print("   ");

  lcd.setCursor(0, 1);
  lcd.print("Toll:");
  lcd.print(tollCollected);
  lcd.print(" Rs   ");
   if (gateStatus == "Open") {
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(RED_LED, HIGH);
    } 
    else if (gateStatus == "Closed"){
       digitalWrite(GREEN_LED, HIGH);
       digitalWrite(RED_LED, LOW);
  }
}
// ---------------- FULL Message ----------------
// ---------------- Toll Message ----------------
void showFullMessage() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Payment Done");

  lcd.setCursor(0, 1);
  lcd.print("Gate Opening");

  delay(1000);

  lcd.clear();
}
// ---------------- JSON API ----------------
void handleData() {

  String json = "{";

  json += "\"vehicles\":" + String(vehiclesPassed) + ",";
  json += "\"toll\":" + String(tollCollected) + ",";
  json += "\"gate\":\"" + gateStatus + "\"";
  json += ",\"entry\":\"" + String(digitalRead(IR_ENTRY)==LOW ? "Detected":"Idle") + "\"";
  json += ",\"exit\":\"" + String(digitalRead(IR_EXIT)==LOW ? "Detected":"Idle") + "\"";

  json += "}";

  server.send(200, "application/json", json);
}
String getHTML() {

String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Toll Gate System</title>
    <!-- Tailwind CSS CDN -->
    <script src="https://cdn.tailwindcss.com"></script>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap" rel="stylesheet">
    <style>
        body {
            font-family: 'Inter', sans-serif;
        }
    </style>
</head>

<body class="bg-slate-950 text-slate-100 min-h-screen flex flex-col justify-between p-6 sm:p-10">

    <!-- Top Navigation / Header Dashboard style -->
    <header class="max-w-6xl w-full mx-auto flex flex-col sm:flex-row items-center justify-between border-b border-slate-800 pb-6 mb-8 gap-4">
        <div>
            <h1 class="text-3xl font-extrabold tracking-tight text-transparent bg-clip-text bg-gradient-to-r from-sky-400 to-emerald-400">
                🚦 Smart Toll Gate
            </h1>
            <p class="text-sm text-slate-400 mt-1">Real-time IoT Monitoring Dashboard</p>
        </div>
        <div class="flex items-center gap-2 bg-slate-900 border border-slate-800 px-4 py-2 rounded-full text-xs font-medium text-sky-400 uppercase tracking-wider">
            <span class="w-2 h-2 rounded-full bg-emerald-500 animate-pulse"></span>
            System Live
        </div>
    </header>

    <!-- Main Dashboard Grid -->
    <main class="max-w-6xl w-full mx-auto grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-6 flex-grow">
        
        <!-- Vehicles Card -->
        <div class="bg-slate-900/60 backdrop-blur-md border border-slate-800/80 rounded-2xl p-6 flex flex-col justify-between transition-all hover:border-slate-700">
            <div class="flex items-center justify-between">
                <span class="text-sm font-semibold tracking-wide uppercase text-slate-400">Total Vehicles</span>
                <span class="p-2 bg-emerald-500/10 text-emerald-400 rounded-xl text-xl">🚗</span>
            </div>
            <div id="vehicles" class="text-5xl font-bold text-emerald-400 mt-6 tracking-tight">0</div>
        </div>

        <!-- Toll Card -->
        <div class="bg-slate-900/60 backdrop-blur-md border border-slate-800/80 rounded-2xl p-6 flex flex-col justify-between transition-all hover:border-slate-700">
            <div class="flex items-center justify-between">
                <span class="text-sm font-semibold tracking-wide uppercase text-slate-400">Toll Collected</span>
                <span class="p-2 bg-amber-500/10 text-amber-400 rounded-xl text-xl">💰</span>
            </div>
            <div id="toll" class="text-5xl font-bold text-amber-400 mt-6 tracking-tight">Rs 0</div>
        </div>

        <!-- Gate Status Card -->
        <div class="bg-slate-900/60 backdrop-blur-md border border-slate-800/80 rounded-2xl p-6 flex flex-col justify-between transition-all hover:border-slate-700">
            <div class="flex items-center justify-between">
                <span class="text-sm font-semibold tracking-wide uppercase text-slate-400">Gate Barrier</span>
                <span class="p-2 bg-slate-800 text-slate-300 rounded-xl text-xl">🚪</span>
            </div>
            <div id="gate" class="text-5xl font-bold text-rose-500 mt-6 tracking-tight">Closed</div>
        </div>

        <!-- Entry IR Card -->
        <div id="entry-card" class="bg-slate-900/60 backdrop-blur-md border border-slate-800/80 rounded-2xl p-6 flex flex-col justify-between transition-all hover:border-slate-700">
            <div class="flex items-center justify-between">
                <span class="text-sm font-semibold tracking-wide uppercase text-slate-400">Entry IR Sensor</span>
                <span class="p-2 bg-sky-500/10 text-sky-400 rounded-xl text-xl">📍</span>
            </div>
            <div id="entry" class="text-5xl font-bold text-sky-400 mt-6 tracking-tight">Idle</div>
        </div>

        <!-- Exit IR Card -->
        <div id="exit-card" class="bg-slate-900/60 backdrop-blur-md border border-slate-800/80 rounded-2xl p-6 flex flex-col justify-between transition-all hover:border-slate-700">
            <div class="flex items-center justify-between">
                <span class="text-sm font-semibold tracking-wide uppercase text-slate-400">Exit IR Sensor</span>
                <span class="p-2 bg-sky-500/10 text-sky-400 rounded-xl text-xl">📍</span>
            </div>
            <div id="exit" class="text-5xl font-bold text-sky-400 mt-6 tracking-tight">Idle</div>
        </div>

        <!-- Traffic Light Card -->
        <div class="bg-slate-900/60 backdrop-blur-md border border-slate-800/80 rounded-2xl p-6 flex flex-col justify-between transition-all hover:border-slate-700">
            <div class="flex items-center justify-between">
                <span class="text-sm font-semibold tracking-wide uppercase text-slate-400">Traffic Signal</span>
                <span class="p-2 bg-slate-800 text-slate-300 rounded-xl text-xl">🚦</span>
            </div>
            <div id="traffic" class="text-5xl font-bold text-rose-500 mt-6 tracking-tight">STOP</div>
        </div>

    </main>

    <!-- Footer -->
    <footer class="max-w-6xl w-full mx-auto text-center text-xs text-slate-600 mt-12 pt-4 border-t border-slate-900">
        ESP32 Web Server &bull; Tailwind Dashboard UI
    </footer>

    <!-- Scripts and Dashboard Live Logic -->
    <script>
        async function updateData(){
            try {
                const response = await fetch('/data');
                const data = await response.json();

                document.getElementById("vehicles").innerText = data.vehicles;
                document.getElementById("toll").innerText = "Rs " + data.toll;
                document.getElementById("gate").innerText = data.gate;
                document.getElementById("entry").innerText = data.entry;
                document.getElementById("exit").innerText = data.exit;

                // Dynamic Logic for Gate and Traffic Lights
                if(data.gate === "Open"){
                    document.getElementById("traffic").innerText = "GO";
                    document.getElementById("traffic").className = "text-5xl font-bold text-emerald-400 mt-6 tracking-tight";
                    document.getElementById("gate").className = "text-5xl font-bold text-emerald-400 mt-6 tracking-tight";
                } else {
                    document.getElementById("traffic").innerText = "STOP";
                    document.getElementById("traffic").className = "text-5xl font-bold text-rose-500 mt-6 tracking-tight";
                    document.getElementById("gate").className = "text-5xl font-bold text-rose-500 mt-6 tracking-tight";
                }

                // Dynamic UI states when Entry/Exit IR beams are broken
                if(data.entry === "Detected") {
                    document.getElementById("entry").className = "text-5xl font-bold text-amber-400 mt-6 tracking-tight animate-pulse";
                    document.getElementById("entry-card").className = "bg-amber-950/20 backdrop-blur-md border border-amber-500/50 rounded-2xl p-6 flex flex-col justify-between transition-all";
                } else {
                    document.getElementById("entry").className = "text-5xl font-bold text-sky-400 mt-6 tracking-tight";
                    document.getElementById("entry-card").className = "bg-slate-900/60 backdrop-blur-md border border-slate-800/80 rounded-2xl p-6 flex flex-col justify-between transition-all hover:border-slate-700";
                }

                if(data.exit === "Detected") {
                    document.getElementById("exit").className = "text-5xl font-bold text-amber-400 mt-6 tracking-tight animate-pulse";
                    document.getElementById("exit-card").className = "bg-amber-950/20 backdrop-blur-md border border-amber-500/50 rounded-2xl p-6 flex flex-col justify-between transition-all";
                } else {
                    document.getElementById("exit").className = "text-5xl font-bold text-sky-400 mt-6 tracking-tight";
                    document.getElementById("exit-card").className = "bg-slate-900/60 backdrop-blur-md border border-slate-800/80 rounded-2xl p-6 flex flex-col justify-between transition-all hover:border-slate-700";
                }

            } catch(err) {
                console.log(err);
            }
        }

        setInterval(updateData, 1000);
        updateData();
    </script>

</body>
</html>
)rawliteral";

return html;
}

// ---------------- Open Gate ----------------
void openGate() {

  gateStatus = "Open";

  // OPEN = GREEN
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, HIGH);

  for (int pos = 90; pos >= 0; pos--) {
    barrierServo.write(pos);
    delay(2);
  }

  gateTimer = millis();
  gateOpen = true;

  updateLCD();
}

// ---------------- Close Gate ----------------
void closeGate() {

  for (int pos = 0; pos <= 90; pos++) {
    barrierServo.write(pos);
    delay(2);
  }

  gateStatus = "Closed";

  // CLOSED = RED
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);

  gateOpen = false;

  updateLCD();
}

void shortBeep() {
  digitalWrite(BUZZER, HIGH);
  delay(100);
  digitalWrite(BUZZER, LOW);
}

void fullAlert() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER, HIGH);
    delay(200);
    digitalWrite(BUZZER, LOW);
    delay(200);
  }
}
void setup() {

  Serial.begin(115200);

  pinMode(IR_ENTRY, INPUT);
  pinMode(IR_EXIT, INPUT);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(BUZZER, LOW);

  // Servo setup
  barrierServo.setPeriodHertz(50);
  barrierServo.attach(SERVO_PIN, 500, 2400);

  // Servo test
  barrierServo.write(90);   // Closed
  delay(1000);

  barrierServo.write(0);    // Open
  delay(1000);

  barrierServo.write(90);   // Back Closed

  // LCD setup
  Wire.begin(21,22);
  lcd.init();
  lcd.noBacklight();
delay(1000);
lcd.backlight();
  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("Starting...");
  delay(2000);
  // Initial gate closed = RED
  gateStatus = "Closed";
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);

  lcd.setCursor(0,0);
  lcd.print("Connecting WiFi");

  // WiFi connect
  WiFi.begin(ssid, password);
  
  int timeout = 20;

  while (WiFi.status() != WL_CONNECTED && timeout > 0) {
    delay(500);
    Serial.print(".");
    timeout--;
  }

  lcd.clear();

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("");
    Serial.println("WiFi Connected");
    Serial.println(WiFi.localIP());

    lcd.setCursor(0,0);     
    lcd.print("WiFi Connected");
    lcd.setCursor(0,1);
    lcd.print(WiFi.localIP());

  } else {

    Serial.println("WiFi Failed");

    lcd.setCursor(0,0);
    lcd.print("WiFi Failed");
  }

  delay(2000);

  // Final idle state
  lcd.clear();
  gateStatus = "Closed";
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);
  updateLCD();

  // Web server routes
  server.on("/", []() {
    server.send(200, "text/html", getHTML());
  });

  server.on("/data", handleData);

  server.begin();

  Serial.println("Server Started");
}
//--------------- Loop ----------------
void loop() {
  server.handleClient();

  // ---------------- ENTRY ----------------
  if (digitalRead(IR_ENTRY) == LOW && !entryTriggered) {

    entryTriggered = true;

    tollCollected += tollPerVehicle;
    

    shortBeep();
    showFullMessage();
    openGate();   
}

  if (digitalRead(IR_ENTRY) == HIGH) {

    entryTriggered = false;
  }

  // ---------------- EXIT ----------------
  if (digitalRead(IR_EXIT) == LOW && !exitTriggered) {

    exitTriggered = true;

    vehiclesPassed++;

    shortBeep();

     Serial.println("Car Exited");

    // Keep RED for exit
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);

    updateLCD();
}

if (digitalRead(IR_EXIT) == HIGH && exitTriggered) {

    exitTriggered = false;
}

  // ---------------- AUTO CLOSE GATE ----------------
  if (gateOpen && millis() - gateTimer >= 700) {

    closeGate();
  }
}
