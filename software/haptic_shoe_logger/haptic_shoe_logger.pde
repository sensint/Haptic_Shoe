import controlP5.*;
import java.util.*;
import processing.serial.*;


// ====================== Defaults =============================
List experiencesList = Arrays.asList(
  "default",
  "dense (20 gr) - M1",
  "dense (50 gr) - M1",
  "cont. vib. (top) - M1",
  "cont. vib. (bot) - M1",
  "sparse (5 gr) - M1",
  "dense (20 gr) - M3",
  "dense (50 gr) - M3",
  "cont. vib. (top) - M3",
  "cont. vib. (bot) - M3",
  "sparse (5 gr) - M3",
  "random (20 gr)"
  );
int experience_left = 0;
int experience_right = 0;
boolean augmentation_left = true;
boolean augmentation_right = true;
int sensorLimitBottom = 0;
int sensorLimitTop = 600;
int recordInterval = 50;
String fileName = "shoe_data";

// ====================== GUI Controls =============================
ControlP5 cp5;
Chart leftShoeFsrChart;
Chart rightShoeFsrChart;
Textlabel recTimeLabel;
ControlTimer timer;

// ====================== GUI Layout =============================
int colWidth = 200;
int colSpace = 10;
int col1X = colSpace;
int col2X = colWidth + 2*colSpace;
int col3X = 2*colWidth + 3*colSpace;
int rowHeight = 20;
int rowSpace = 10;
int row1Y = rowSpace;
int row2Y = 2*rowHeight + rowSpace;
int row3Y = row2Y + 60;
int row4Y = row3Y + rowHeight + 2*rowSpace;
int row5Y = row4Y + 2*rowHeight + rowSpace;
int row6Y = row5Y + 60;
int row7Y = row6Y + rowHeight + rowSpace;
int dataVizHeight = 5*rowHeight + rowSpace;
int visNumDataPoints = 100;

// ====================== Colors =============================
// https://colorhunt.co/palette/2a09443fa796fec260a10035
color colorLabelsLight = color(255);
color colorLabelsDark = color(0);
color colorBackgroundLight = color(240);
color colorVTop = color(42, 9, 68);
color colorVBottom = color(63, 167, 150);
color colorHInner = color(254, 194, 96);
color colorHOuter = color(161, 0, 53);


// ====================== I/O =============================
Serial serialPort;
String serialPortName = "";
boolean serialPortConnected = false;

PrintWriter output;
int fileIterator = 1;
boolean recordActive = false;


boolean initialized = false;


// ====================== FSR =============================
int fsrLeftVT = 0;
int fsrLeftVB = 0;
int fsrLeftHI = 0;
int fsrLeftHO = 0;
int fsrRightVT = 0;
int fsrRightVB = 0;
int fsrRightHI = 0;
int fsrRightHO = 0;

void setup() {
  size(640, 320);
  frameRate(300);

  cp5 = new ControlP5(this);
  cp5.setColor(ControlP5.THEME_RETRO);


  // ====================== Augmentation =============================

  cp5.addToggle("augmentation_left")
     .setPosition(col1X, row1Y)
     .setSize(colWidth/2, rowHeight)
     .setColorLabel(colorLabelsDark)
     .setValue(augmentation_left)
     .setMode(ControlP5.SWITCH)
     ;
  cp5.addToggle("augmentation_right")
     .setPosition(col2X, row1Y)
     .setSize(colWidth/2, rowHeight)
     .setColorLabel(colorLabelsDark)
     .setValue(augmentation_right)
     .setMode(ControlP5.SWITCH)
     ;
  cp5.addToggle("augmentation")
     .setPosition(col3X, row1Y)
     .setSize(colWidth/2, rowHeight)
     .setColorLabel(colorLabelsDark)
     .setValue(augmentation_left)
     .setMode(ControlP5.SWITCH)
     ;


  // ====================== Recording =============================

  cp5.addTextfield("filename")
     .setPosition(col3X, row3Y)
     .setSize(colWidth, rowHeight)
     .setAutoClear(false)
     .setColorLabel(colorLabelsDark)
     .setValue(fileName)
     ;

  cp5.addSlider("rec_interval")
     .setPosition(col3X, row4Y)
     .setSize(colWidth, rowHeight)
     .setRange(10,100)
     .setValue(recordInterval)
     .setNumberOfTickMarks(10)
     .setDecimalPrecision(0)
     .setSliderMode(Slider.FLEXIBLE)
     .setColorLabel(colorLabelsDark)
     .setColorValue(colorLabelsDark)
     ;
  cp5.getController("rec_interval").getValueLabel().align(ControlP5.LEFT, ControlP5.BOTTOM_OUTSIDE)
    .setPaddingX(0)
    .setPaddingY(10)
    ;
  cp5.getController("rec_interval").getCaptionLabel().align(ControlP5.RIGHT, ControlP5.BOTTOM_OUTSIDE)
    .setPaddingX(0)
    .setPaddingY(10)
    ;

  cp5.addToggle("recording")
     .setPosition(col3X, row5Y)
     .setSize(colWidth/2, rowHeight)
     .setColorLabel(colorLabelsDark)
     .setValue(false)
     .setMode(ControlP5.SWITCH)
     ;

  timer = new ControlTimer();
  recTimeLabel = new Textlabel(cp5, "--", col3X+colWidth/2+colSpace, row5Y+rowHeight/3);
  recTimeLabel.setColor(colorLabelsDark);
  timer.setSpeedOfTime(1);


  // ====================== Data Visualization =============================

  leftShoeFsrChart = cp5.addChart("fsr_left")
               .setPosition(col1X, row3Y)
               .setSize(colWidth, dataVizHeight)
               .setRange(sensorLimitBottom, sensorLimitTop)
               .setView(Chart.LINE)
               .setStrokeWeight(1.5)
               .setColorCaptionLabel(colorLabelsDark)
               .setColorBackground(colorBackgroundLight)
               ;
  leftShoeFsrChart.addDataSet("v_top");
  leftShoeFsrChart.setData("v_top", new float[visNumDataPoints]);
  leftShoeFsrChart.setColors("v_top", colorVTop);
  leftShoeFsrChart.addDataSet("v_bottom");
  leftShoeFsrChart.setData("v_bottom", new float[visNumDataPoints]);
  leftShoeFsrChart.setColors("v_bottom", colorVBottom);
  leftShoeFsrChart.addDataSet("h_inner");
  leftShoeFsrChart.setData("h_inner", new float[visNumDataPoints]);
  leftShoeFsrChart.setColors("h_inner", colorHInner);
  leftShoeFsrChart.addDataSet("h_outer");
  leftShoeFsrChart.setData("h_outer", new float[visNumDataPoints]);
  leftShoeFsrChart.setColors("h_outer", colorHOuter);

  rightShoeFsrChart = cp5.addChart("fsr_right")
               .setPosition(col2X, row3Y)
               .setSize(colWidth, dataVizHeight)
               .setRange(sensorLimitBottom, sensorLimitTop)
               .setView(Chart.LINE)
               .setStrokeWeight(1.5)
               .setColorCaptionLabel(colorLabelsDark)
               .setColorBackground(colorBackgroundLight)
               ;
  rightShoeFsrChart.addDataSet("v_top");
  rightShoeFsrChart.setData("v_top", new float[visNumDataPoints]);
  rightShoeFsrChart.setColors("v_top", colorVTop);
  rightShoeFsrChart.addDataSet("v_bottom");
  rightShoeFsrChart.setData("v_bottom", new float[visNumDataPoints]);
  rightShoeFsrChart.setColors("v_bottom", colorVBottom);
  rightShoeFsrChart.addDataSet("h_inner");
  rightShoeFsrChart.setData("h_inner", new float[visNumDataPoints]);
  rightShoeFsrChart.setColors("h_inner", colorHInner);
  rightShoeFsrChart.addDataSet("h_outer");
  rightShoeFsrChart.setData("h_outer", new float[visNumDataPoints]);
  rightShoeFsrChart.setColors("h_outer", colorHOuter);


  // ====================== Experience =============================

  cp5.addScrollableList("experience_left")
     .setPosition(col1X, row2Y)
     .setSize(colWidth/2, 120)
     .setBarHeight(rowHeight)
     .setItemHeight(rowHeight)
     .addItems(experiencesList)
     .setOpen(false)
     ;
  cp5.addButton("experience_left_btn")
     .setValue(0)
     .setPosition(col1X + colWidth/2 + colSpace, row2Y)
     .setSize(colWidth/2 - colSpace, rowHeight)
     .setLabel("send")
     ;

  cp5.addScrollableList("experience_right")
     .setPosition(col2X, row2Y)
     .setSize(colWidth/2, 120)
     .setBarHeight(rowHeight)
     .setItemHeight(rowHeight)
     .addItems(experiencesList)
     .setOpen(false)
     ;
  cp5.addButton("experience_right_btn")
     .setValue(0)
     .setPosition(col2X + colWidth/2 + colSpace, row2Y)
     .setSize(colWidth/2 - colSpace, rowHeight)
     .setLabel("send")
     ;

  cp5.addScrollableList("experience")
     .setPosition(col3X, row2Y)
     .setSize(colWidth/2, 120)
     .setBarHeight(rowHeight)
     .setItemHeight(rowHeight)
     .addItems(experiencesList)
     .setOpen(false)
     ;
  cp5.addButton("experience_btn")
     .setValue(0)
     .setPosition(col3X + colWidth/2 + colSpace, row2Y)
     .setSize(colWidth/2 - colSpace, rowHeight)
     .setLabel("send")
     ;


  // ====================== IMU =============================

  cp5.addButton("reset_imu_btn")
     .setValue(0)
     .setPosition(col3X, row7Y)
     .setSize((int)(colWidth*0.5), rowHeight)
     .setLabel("reset IMUs")
     ;


  // ====================== Serial =============================

  cp5.addScrollableList("serial_port")
     .setPosition(col3X, row6Y)
     .setSize((int)(colWidth*0.8), 60)
     .setBarHeight(rowHeight)
     .setItemHeight(rowHeight)
     .addItems(Serial.list())
     .setOpen(false)
     ;
  cp5.addButton("serial_port_btn")
     .setValue(0)
     .setPosition(col3X+(int)(colWidth*0.8), row6Y)
     .setSize((int)(colWidth*0.2), rowHeight)
     .setLabel("connect")
     ;

  initialized = true;
}


void draw() {
  background(200);
  //println(frameRate);

  if (recordActive) {
    recTimeLabel.setValue(timer.toString());
  } else {
    recTimeLabel.setValue("00 : 00 : 00");
  }
  recTimeLabel.draw(this);
}


void experience_left(int n) {
  if (!initialized) {
    return;
  }
  experience_left = n;
  println("select new experience for left shoe: " + n);
}


void experience_left_btn(int val) {
  if (!initialized || !serialPortConnected) {
    return;
  }
  serialPort.write("<6,34,1," + experience_left + ">");
  println("change experience on left shoe: " + experience_left);
}


void experience_right(int n) {
  if (!initialized) {
    return;
  }
  experience_right = n;
  println("select new experience for right shoe: " + n);
}


void experience_right_btn(int val) {
  if (!initialized || !serialPortConnected) {
    return;
  }
  serialPort.write("<7,34,1," + experience_right + ">");
  println("change experience on right shoe: " + experience_right);
}


void experience(int n) {
  if (!initialized) {
    return;
  }
  experience_left = n;
  experience_right = n;
  println("select new experience for both shoes: " + n);
  cp5.get(ScrollableList.class, "experience_left").setValue(n);
  cp5.get(ScrollableList.class, "experience_right").setValue(n);
}


void experience_btn(int val) {
  if (!initialized || !serialPortConnected) {
    return;
  }
  serialPort.write("<6,34,1," + experience_left + ">");
  serialPort.write("<7,34,1," + experience_right + ">");
  println("change experience on both shoes: " + experience_left + "|" + experience_right);
}


void serial_port(int n) {
  if (!initialized) {
    return;
  }
  serialPortName = cp5.get(ScrollableList.class, "serial_port").getItem(n).get("name").toString();
  println(n, serialPortName);
}


void serial_port_btn(int val) {
  if (!initialized) {
    return;
  }
  try {
    serialPort = new Serial(this, serialPortName, 115200);
  } catch (Exception e) {
    println("could not connect to serial port " + serialPortName);
    return;
  }
  serialPort.bufferUntil('>');
  serialPort.clear();
  serialPortConnected = true;
  println("connect to serial port: " + serialPortName);
}


void augmentation_left(boolean active) {
  if (!initialized || !serialPortConnected) {
    return;
  }
  augmentation_left = active;
  if (active) {
    serialPort.write("<6,32,0,->");
  } else {
    serialPort.write("<6,33,0,->");
  }
  println(active ? "left activated" : "left deactivated");
}


void augmentation_right(boolean active) {
  if (!initialized || !serialPortConnected) {
    return;
  }
  augmentation_right = active;
  if (active) {
    serialPort.write("<7,32,0,->");
  } else {
    serialPort.write("<7,33,0,->");
  }
  println(active ? "right activated" : "right deactivated");
}


void augmentation(boolean active) {
  if (!initialized || !serialPortConnected) {
    return;
  }
  augmentation_left = active;
  augmentation_right = active;
  cp5.get(Toggle.class, "augmentation_left").setValue(active);
  cp5.get(Toggle.class, "augmentation_right").setValue(active);
  if (active) {
    serialPort.write("<0,32,0,->");
  } else {
    serialPort.write("<0,33,0,->");
  }
  println(active ? "both activated" : "both deactivated");
}


void recording(boolean active) {
  if (!initialized || !serialPortConnected) {
    return;
  }
  println(active ? "recording started" : "recording stopped");
  recordActive = active;

  if (active) {
    String path = dataPath(fileName + "_" + fileIterator +".csv");
    if (dataFile(path).exists()) {
      path = dataPath(fileName + "_" + (++fileIterator) +".csv");
    } else {
      fileIterator = 1;
    }
    output = createWriter(path);
    output.println("timestamp,shoeL_on,shoeR_on,shoeL_experience,shoeR_experience,shoeL_fsr_VT,shoeL_fsr_VB,shoeL_fsr_HI,shoeL_fsr_HO,shoeL_IMU_rot_W,shoeL_IMU_rot_X,shoeL_IMU_rot_Y,shoeL_IMU_rot_Z,shoeL_IMU_acc_X,shoeL_IMU_acc_Y,shoeL_IMU_acc_Z,shoeL_IMU_calib,shoeL_IMU_dt,shoeR_fsr_VT,shoeR_fsr_VB,shoeR_fsr_HI,shoeR_fsr_HO,shoeR_IMU_rot_W,shoeR_IMU_rot_X,shoeR_IMU_rot_Y,shoeR_IMU_rot_Z,shoeR_IMU_acc_X,shoeR_IMU_acc_Y,shoeR_IMU_acc_Z,shoeR_IMU_calib,shoeR_IMU_dt");
    timer.reset();
    serialPort.write("<0,18,1," + recordInterval + ">");
    serialPort.write("<2,16,0,->");
  } else {
    serialPort.write("<2,17,0,->");
    output.flush();
    output.close();

    leftShoeFsrChart.setData("v_top", new float[visNumDataPoints]);
    leftShoeFsrChart.setData("v_bottom", new float[visNumDataPoints]);
    leftShoeFsrChart.setData("h_inner", new float[visNumDataPoints]);
    leftShoeFsrChart.setData("h_outer", new float[visNumDataPoints]);
    rightShoeFsrChart.setData("v_top", new float[visNumDataPoints]);
    rightShoeFsrChart.setData("v_bottom", new float[visNumDataPoints]);
    rightShoeFsrChart.setData("h_inner", new float[visNumDataPoints]);
    rightShoeFsrChart.setData("h_outer", new float[visNumDataPoints]);
  }
}


void rec_interval(float val) {
  if (!initialized) {
    return;
  }
  recordInterval = (int)val;
  println("recording interval " + val);
}


void serialEvent(Serial port) {
  String data = match(port.readString(), "<(.*?)>")[1];
  String[] tokens = split(data, ',');
  if (parseInt(tokens[1]) != 68) {
    return;
  }
  String logMsg = timer.time() + ","
                + augmentation_left + ","
                + augmentation_right + ","
                + experience_left + ","
                + experience_right + ","
                + join(subset(tokens, 3), ',');
  output.println(logMsg);

  fsrLeftVT = (parseInt(tokens[3]) <= sensorLimitTop) ? parseInt(tokens[3]) : fsrLeftVT;
  fsrLeftVB = (parseInt(tokens[4]) <= sensorLimitTop) ? parseInt(tokens[4]) : fsrLeftVB;
  fsrLeftHI = (parseInt(tokens[5]) <= sensorLimitTop) ? parseInt(tokens[5]) : fsrLeftHI;
  fsrLeftHO = (parseInt(tokens[6]) <= sensorLimitTop) ? parseInt(tokens[6]) : fsrLeftHO;
  fsrRightVT = (parseInt(tokens[16]) <= sensorLimitTop) ? parseInt(tokens[16]) : fsrRightVT;
  fsrRightVB = (parseInt(tokens[17]) <= sensorLimitTop) ? parseInt(tokens[17]) : fsrRightVB;
  fsrRightHI = (parseInt(tokens[18]) <= sensorLimitTop) ? parseInt(tokens[18]) : fsrRightHI;
  fsrRightHO = (parseInt(tokens[19]) <= sensorLimitTop) ? parseInt(tokens[19]) : fsrRightHO;

  leftShoeFsrChart.push("v_top", fsrLeftVT);
  leftShoeFsrChart.push("v_bottom", fsrLeftVB);
  leftShoeFsrChart.push("h_inner", fsrLeftHI);
  leftShoeFsrChart.push("h_outer", fsrLeftHO);
  rightShoeFsrChart.push("v_top", fsrRightVT);
  rightShoeFsrChart.push("v_bottom", fsrRightVB);
  rightShoeFsrChart.push("h_inner", fsrRightHI);
  rightShoeFsrChart.push("h_outer", fsrRightHO);
}


public void filename(String val) {
  fileName = val;
}


void reset_imu_btn(int val) {
  if (!initialized || !serialPortConnected) {
    return;
  }
  serialPort.write("<2,69,0,->");
  println("reset IMUs");
}
