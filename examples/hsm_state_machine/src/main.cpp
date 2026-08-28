#include <Arduino.h>
#include <memory>

#include "hsmcpp/hsm.hpp"
#include "hsmcpp/HsmEventDispatcherArduino.hpp"

using namespace hsmcpp;

// ตัวอย่างนี้กำหนด "สถานะ" ของระบบและ "เหตุการณ์" ที่คอยเปลี่ยนสถานะ
// StateID = สถานะหลักและสถานะย่อยของระบบ
// EventID = เหตุการณ์ที่กระตุ้นให้เปลี่ยนสถานะ เช่น เปิด/ปิด/เริ่มเคลื่อนที่/หยุด
// พื้นฐานของ Hierarchical State Machine (HSM) คือ:
// - ระบบอยู่ในสถานะหนึ่ง ๆ เสมอ
// - เมื่อเกิด event บางอย่าง ระบบอาจเปลี่ยนสถานะ
// - สถานะย่อย (substate) สามารถถูกจัดกลุ่มภายใต้สถานะหลักได้

// สร้าง enum สำหรับสถานะและเหตุการณ์ที่ใช้ควบคุมระบบ
// OFF = ปิดเครื่อง / ไม่ทำงาน
// ACTIVE = เปิดเครื่องแล้ว / ระบบพร้อมทำงาน
// IDLE = อยู่ในโหมดรอ / ไม่เคลื่อนที่
// MOVING = กำลังเคลื่อนที่
// TURN_ON / TURN_OFF / MOVE / STOP = เหตุการณ์ที่กระตุ้นให้ state เปลี่ยน
// ส่วนนี้ก็คือ "โมเดลของระบบ" ที่ HSM จะใช้ในการตัดสินใจว่าเมื่อเกิด event ใด ต้องไป state ไหน
enum class StateID { OFF, ACTIVE, IDLE, MOVING };
enum class EventID { TURN_ON, TURN_OFF, MOVE, STOP };

// Dispatcher ทำหน้าที่จัดการ event queue และเรียก callback ของ FSM ใน loop()
// FSM คือ HierarchicalStateMachine ที่เก็บ state graph และ transition logic
// กล่าวง่าย ๆ คือ ตัว dispatcher จะคอยรอ event จากภายนอก แล้วส่ง event เข้าไปให้ FSM ประมวลผล
std::shared_ptr<HsmEventDispatcherArduino> dispatcher;
std::shared_ptr<HierarchicalStateMachine> fsm;

const int LED_PIN = LED_BUILTIN;

void setup() {
    // ตั้งค่าพอร์ต Serial สำหรับ debug และไฟ LED เป็น output
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);

    // 1) สร้าง Dispatcher สำหรับ Arduino
    // Dispatcher นี้จะช่วยให้ FSM รอ event จากภายนอกและประมวลผลใน loop()
    dispatcher = HsmEventDispatcherArduino::create();

    // 2) สร้าง State Machine ซึ่งเริ่มต้นอยู่ในสถานะ OFF
    // สถานะ OFF เป็น state หลักที่เริ่มต้นของระบบ
    fsm = std::make_shared<HierarchicalStateMachine>(static_cast<StateID_t>(StateID::OFF));

    // 3) ลงทะเบียน state ต่าง ๆ
    // registerState(state, onStateChanged, onEntering, onExiting)
    // callback onEntering จะถูกเรียกเมื่อเข้าสู่ state นั้น ๆ
    // ตัวอย่างนี้ใช้ callback เพียงเพื่อ log และควบคุม LED
    // โดยรวมแล้ว ตัวอย่างนี้แสดงให้เห็นว่า: เมื่อระบบเข้าสู่แต่ละ state จะทำอะไรบ้าง
    // เช่น OFF => ไฟ LED ปิด, ACTIVE => ไฟ LED เปิด, IDLE => พิมพ์ว่าหยุดนิ่ง, MOVING => พิมพ์ว่ากำลังเคลื่อนที่
    fsm->registerState(static_cast<StateID_t>(StateID::OFF),
                       nullptr,
                       [](const VariantVector_t&) {
                           Serial.println("[STATE] OFF -> ปิดระบบ");
                           digitalWrite(LED_PIN, LOW);
                           return true;
                       },
                       nullptr);

    fsm->registerState(static_cast<StateID_t>(StateID::ACTIVE),
                       nullptr,
                       [](const VariantVector_t&) {
                           Serial.println("[STATE] ACTIVE -> ระบบพร้อมทำงาน");
                           digitalWrite(LED_PIN, HIGH);
                           return true;
                       },
                       nullptr);

    fsm->registerState(static_cast<StateID_t>(StateID::IDLE),
                       nullptr,
                       [](const VariantVector_t&) {
                           Serial.println("   [SUB-STATE] IDLE -> มอเตอร์หยุดนิ่ง");
                           return true;
                       },
                       nullptr);

    fsm->registerState(static_cast<StateID_t>(StateID::MOVING),
                       nullptr,
                       [](const VariantVector_t&) {
                           Serial.println("   [SUB-STATE] MOVING -> มอเตอร์กำลังหมุน...");
                           return true;
                       },
                       nullptr);

    // สร้าง hierarchy ของ state
    // ACTIVE เป็น parent state, IDLE และ MOVING เป็น child state
    // ความหมายคือ เมื่อระบบอยู่ใน ACTIVE แล้ว state ที่ใช้งานจริงจะเป็น IDLE หรือ MOVING
    // ตัวอย่างนี้จึงเป็นการแสดงแนวคิดแบบ Hierarchical State Machine ที่ซ้อน state ไว้ใน state หลัก
    fsm->registerSubstate(static_cast<StateID_t>(StateID::ACTIVE), static_cast<StateID_t>(StateID::IDLE));
    fsm->registerSubstate(static_cast<StateID_t>(StateID::ACTIVE), static_cast<StateID_t>(StateID::MOVING));

    // กำหนด transition หลัก ๆ ของระบบ
    // OFF --TURN_ON--> IDLE
    // IDLE --MOVE--> MOVING
    // MOVING --STOP--> IDLE
    // ACTIVE --TURN_OFF--> OFF
    // สิ่งนี้แปลว่า: ระบบจะเปลี่ยนสถานะตามเหตุการณ์ที่เกิดขึ้น เช่น กดปุ่ม 1,0,2,3
    fsm->registerTransition(static_cast<StateID_t>(StateID::OFF),
                            static_cast<StateID_t>(StateID::IDLE),
                            static_cast<EventID_t>(EventID::TURN_ON));

    fsm->registerTransition(static_cast<StateID_t>(StateID::ACTIVE),
                            static_cast<StateID_t>(StateID::OFF),
                            static_cast<EventID_t>(EventID::TURN_OFF));

    fsm->registerTransition(static_cast<StateID_t>(StateID::IDLE),
                            static_cast<StateID_t>(StateID::MOVING),
                            static_cast<EventID_t>(EventID::MOVE));

    fsm->registerTransition(static_cast<StateID_t>(StateID::MOVING),
                            static_cast<StateID_t>(StateID::IDLE),
                            static_cast<EventID_t>(EventID::STOP));

    // เริ่มใช้งาน HSM ต่อจากนี้
    // initialize() จะทำการผูก dispatcher กับ state machine และตั้ง state เริ่มต้นเป็น OFF
    // จากนั้นทุก event ที่ส่งเข้ามาจะถูกประมวลผลโดย dispatcher ใน loop()
    if (!fsm->initialize(dispatcher)) {
        Serial.println("HSM initialize failed");
    }

    // ตัวอย่างนี้เป็น demo แบบ interactive
    // ผู้ใช้กดพิมพ์ผ่าน Serial Monitor เพื่อสั่งให้ FSM เปลี่ยน state
    // '1' = เปิดระบบ, '0' = ปิดระบบ, '2' = เริ่มเคลื่อนที่, '3' = หยุด
    Serial.println("พิมพ์คำสั่งผ่าน Serial:");
    Serial.println("'1' = เปิด, '0' = ปิด, '2' = วิ่ง, '3' = หยุด");
}

void loop() {
    // loop() เป็นส่วนที่ต้องเรียก dispatcher->dispatchEvents() อยู่เสมอ
    // เพราะ event จะถูกเก็บไว้ใน queue ของ dispatcher ก่อนที่ FSM จะประมวลผล
    dispatcher->dispatchEvents();

    // อ่านคำสั่งจาก Serial Monitor
    // ถ้าพบ '1', '0', '2', '3' จะเป็นการ trigger event ให้ state machine เปลี่ยนสถานะ
    if (Serial.available() > 0) {
        const char cmd = Serial.read();

        switch (cmd) {
            case '1':
                fsm->transition(static_cast<EventID_t>(EventID::TURN_ON));
                break;
            case '0':
                fsm->transition(static_cast<EventID_t>(EventID::TURN_OFF));
                break;
            case '2':
                fsm->transition(static_cast<EventID_t>(EventID::MOVE));
                break;
            case '3':
                fsm->transition(static_cast<EventID_t>(EventID::STOP));
                break;
            default:
                break;
        }
    }
}