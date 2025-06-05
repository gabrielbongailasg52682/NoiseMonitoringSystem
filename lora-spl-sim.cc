#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lorawan-module.h"
#include <sqlite3.h>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <chrono>

using namespace ns3;

Ptr<lorawan::EndDeviceLoraPhy> g_edPhy;
std::ofstream csvFile("sound_log.csv", std::ios::out);
const std::string deviceId = "NMS_0001";
Ptr<UniformRandomVariable> jitter = CreateObject<UniformRandomVariable>();

std::string GetWallClockTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&now_time), "%H:%M:%S %d-%m-%Y");
    return oss.str();
}

void InitializeDatabase(const std::string& deviceId) {
    sqlite3* db;
    int rc = sqlite3_open("simulation_data.db", &db);
    if (rc) return;

    std::string sql = "CREATE TABLE IF NOT EXISTS " + deviceId + "("
                      "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "TIMESTAMP TEXT NOT NULL, "
                      "SOUND_PRESSURE REAL);";
    sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);
    sqlite3_close(db);
}

void InitializeStatusTable(const std::string& deviceId) {
    sqlite3* db;
    int rc = sqlite3_open("simulation_data.db", &db);
    if (rc) return;

    std::string tableName = deviceId + "_Status";
    std::string sql = "CREATE TABLE IF NOT EXISTS " + tableName + "("
                      "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "TIMESTAMP TEXT NOT NULL, "
                      "BATTERY_VOLTAGE REAL, "
                      "BATTERY_CURRENT REAL, "
                      "TEMPERATURE REAL, "
                      "LATITUDE REAL, "
                      "LONGITUDE REAL);";
    sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);
    sqlite3_close(db);
}

void InsertData(const std::string& deviceId, const std::string& timestamp, double soundPressure) {
    sqlite3* db;
    int rc = sqlite3_open("simulation_data.db", &db);
    if (rc) return;

    std::stringstream ss;
    ss << "INSERT INTO " << deviceId
       << " (TIMESTAMP, SOUND_PRESSURE) VALUES ('"
       << timestamp << "', " << soundPressure << ");";
    sqlite3_exec(db, ss.str().c_str(), nullptr, nullptr, nullptr);
    sqlite3_close(db);
}

void InsertStatusData(const std::string& deviceId, const std::string& timestamp,
                      double batteryVoltage, double batteryCurrent, double temperature,
                      double latitude, double longitude) {
    sqlite3* db;
    int rc = sqlite3_open("simulation_data.db", &db);
    if (rc) return;

    std::string tableName = deviceId + "_Status";
    std::stringstream ss;
    ss << "INSERT INTO " << tableName
       << " (TIMESTAMP, BATTERY_VOLTAGE, BATTERY_CURRENT, TEMPERATURE, LATITUDE, LONGITUDE) "
       << "VALUES ('" << timestamp << "', " << batteryVoltage << ", "
       << batteryCurrent << ", " << temperature << ", "
       << latitude << ", " << longitude << ");";
    sqlite3_exec(db, ss.str().c_str(), nullptr, nullptr, nullptr);
    sqlite3_close(db);
}

void SendStatusReport(Ptr<lorawan::EndDeviceLoraPhy> phy, const std::string& deviceId) {
    Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
    double batteryVoltage = rand->GetValue(3.0, 4.2);
    double batteryCurrent = rand->GetValue(0.01, 0.5);
    double temperature = rand->GetValue(20.0, 40.0);
    double lat = 35.873522;
    double lon = 14.521170;
    std::string timestamp = GetWallClockTime();

    std::ostringstream reportStream;
    reportStream << "Status=Battery: " << batteryVoltage << "V, "
                 << batteryCurrent << "A, Temperature: " << temperature
                 << "°C, Location: (" << lat << ", " << lon << ")";
    std::string report = reportStream.str();

    InsertStatusData(deviceId, timestamp, batteryVoltage, batteryCurrent, temperature, lat, lon);

    Ptr<Packet> packet = Create<Packet>((uint8_t*)report.c_str(), report.size());
    lorawan::LoraTxParameters txParams;
    txParams.sf = 7;
    double freq = 868100000 + jitter->GetValue(-20000, 20000);
    phy->Send(packet, txParams, txParams.sf, freq);

    std::cout << "[" << timestamp << "] Node " << deviceId << " sent status report: " << report << std::endl;

    double delay = 30.0 + jitter->GetValue(0.5, 1.5);
    Simulator::Schedule(Seconds(delay), &SendStatusReport, phy, deviceId);
}

void SendSoundData(Ptr<lorawan::EndDeviceLoraPhy> phy, const std::string& deviceId) {
    Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
    double value = rand->GetValue(30.0, 100.0);
    std::string timestamp = GetWallClockTime();

    InsertData(deviceId, timestamp, value);

    std::ostringstream payload;
    payload << timestamp << ", Sound Pressure: " << value << " dB";
    std::string data = payload.str();

    Ptr<Packet> packet = Create<Packet>((uint8_t*)data.c_str(), data.size());
    lorawan::LoraTxParameters txParams;
    txParams.sf = 7;
    double freq = 868100000 + jitter->GetValue(-20000, 20000);
    phy->Send(packet, txParams, txParams.sf, freq);

    std::cout << "[" << timestamp << "] Node " << deviceId << " sent: " << data << std::endl;

    if (csvFile.is_open()) {
        csvFile << "\"" << timestamp << "\"," << value << std::endl;
    }

    double nextDelay = jitter->GetValue(2.0, 3.0);
    Simulator::Schedule(Seconds(nextDelay), &SendSoundData, phy, deviceId);
}

void PollForReportCommand(Ptr<lorawan::EndDeviceLoraPhy> phy, std::string deviceId) {
    std::ifstream file("/home/gabriel/Desktop/Noise Monitoring Prototype/ns3-simulation/ns-3.44/report.flag");
    if (file.is_open()) {
        std::string command;
        std::getline(file, command);
        file.close();

        if (command == "REPORT") {
            std::remove("/home/gabriel/Desktop/Noise Monitoring Prototype/ns3-simulation/ns-3.44/report.flag");
            std::cout << "[Dashboard] Triggered status report via dashboard button.\n";
            SendStatusReport(phy, deviceId);
        }
    }
    Simulator::Schedule(Seconds(1.0), &PollForReportCommand, phy, deviceId);
}

void NodeHandlePacket(Ptr<const Packet> packet, uint32_t sf, Ptr<lorawan::EndDeviceLoraPhy> phy) {
    // No gateway-triggered commands in this version
}

void StaticNodePacketHandler(Ptr<const Packet> packet, uint32_t sf) {
    NodeHandlePacket(packet, sf, g_edPhy);
}

void GatewayReceive(Ptr<const Packet> packet, uint32_t sf) {
    uint32_t size = packet->GetSize();
    uint8_t* buffer = new uint8_t[size];
    packet->CopyData(buffer, size);
    std::string msg((char*)buffer, size);
    delete[] buffer;

    std::cout << "[" << GetWallClockTime() << "] Gateway received (SF" << sf << "): " << msg << std::endl;
}

int main(int argc, char* argv[]) {
    InitializeDatabase(deviceId);
    InitializeStatusTable(deviceId);

    GlobalValue::Bind("SimulatorImplementationType", StringValue("ns3::RealtimeSimulatorImpl"));

    NodeContainer nodes;
    nodes.Create(2);

    MobilityHelper mobility;
    Ptr<ListPositionAllocator> pos = CreateObject<ListPositionAllocator>();
    pos->Add(Vector(0, 0, 0));
    pos->Add(Vector(1000, 0, 10));
    mobility.SetPositionAllocator(pos);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel>();
    loss->SetPathLossExponent(3.76);
    loss->SetReference(1.0, 7.7);
    Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel>();
    Ptr<lorawan::LoraChannel> channel = CreateObject<lorawan::LoraChannel>(loss, delay);

    lorawan::LoraPhyHelper phyHelper;
    phyHelper.SetChannel(channel);
    lorawan::LorawanMacHelper macHelper;
    macHelper.SetRegion(lorawan::LorawanMacHelper::EU);
    lorawan::LoraHelper helper;

    phyHelper.SetDeviceType(lorawan::LoraPhyHelper::ED);
    macHelper.SetDeviceType(lorawan::LorawanMacHelper::ED_A);
    helper.Install(phyHelper, macHelper, nodes.Get(0));

    phyHelper.SetDeviceType(lorawan::LoraPhyHelper::GW);
    macHelper.SetDeviceType(lorawan::LorawanMacHelper::GW);
    helper.Install(phyHelper, macHelper, nodes.Get(1));

    Ptr<lorawan::EndDeviceLoraPhy> edPhy = nodes.Get(0)->GetDevice(0)
        ->GetObject<lorawan::LoraNetDevice>()->GetPhy()->GetObject<lorawan::EndDeviceLoraPhy>();
    Ptr<lorawan::GatewayLoraPhy> gwPhy = nodes.Get(1)->GetDevice(0)
        ->GetObject<lorawan::LoraNetDevice>()->GetPhy()->GetObject<lorawan::GatewayLoraPhy>();

    g_edPhy = edPhy;
    edPhy->TraceConnectWithoutContext("ReceivedPacket", MakeCallback(&StaticNodePacketHandler));
    gwPhy->TraceConnectWithoutContext("ReceivedPacket", MakeCallback(&GatewayReceive));

    Simulator::Schedule(Seconds(2.0), &SendSoundData, edPhy, deviceId);
    Simulator::Schedule(Seconds(5.0), &SendStatusReport, edPhy, deviceId);
    Simulator::Schedule(Seconds(1.0), &PollForReportCommand, edPhy, deviceId);

    Simulator::Run();
    Simulator::Destroy();

    if (csvFile.is_open()) {
        csvFile.close();
    }

    return 0;
}

