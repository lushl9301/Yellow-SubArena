#2014MDPGrp10

#arduino part

##Project Deliverable Checklist Assessment Form

- [x] A2. Sensors calibrated to correctly return distance to obstacle
- [x] A3. Accurate straight line motion
- [ ] A4. Accurate rotation
    - [x] 90 degree rotation
    - [ ] Clock direction rotation (30 degree/mark)
    - [ ] Arbitory angle
- [ ] A5.
    - [x]  Obstacle avoidance
    - [ ]  position recovery
- [ ] A6. Extension beyond the basics
    - [ ] Drifting
    - [ ] ...

##Communication
####Send
```
Serial.println("START");
Serial.print();
Serial.print();
Serial.print();
```
####Get
```
while (!Serial.available() || Serial.read() != 'S');
```

##Command Format
Number and Char
    number for go ahead ```x``` grids
    Char for turning
####example
```
a1 L a2 R R a3
```
