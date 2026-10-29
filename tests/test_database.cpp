#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include "../services/DatabaseService.h"
#include "../models/Request.h"
#include "../models/RescueTeam.h"
#include "../models/Resource.h"
#include "../models/Location.h"
#include "../models/Road.h"
#include "../models/Operation.h"

int main() {
    std::cout << "============================================================\n";
    std::cout << "  SQLITE DATABASE SERVICE UNIT TEST SUITE\n";
    std::cout << "============================================================\n";

    auto& db = services::DatabaseService::getInstance();
    bool openOk = db.openCustom("data/test_disaster.db");
    assert(openOk && "Failed to open isolated test database");
    db.initializeSchema();
    std::cout << "  [PASS] SQLite isolated test database acquired (data/test_disaster.db)\n";

    db.clearAllData();
    assert(db.isDatabaseEmpty() && "Database should be empty after clearAllData");
    std::cout << "  [PASS] Database cleared and confirmed empty\n";

    // 1. Locations
    models::Location l1{"LOC-1", "Depot", "Relief Hub", 100.0, 200.0};
    models::Location l2{"LOC-2", "Sector 4", "Affected Area", 300.0, 400.0};
    assert(db.insertLocation(l1));
    assert(db.insertLocation(l2));
    auto locs = db.loadAllLocations();
    assert(locs.size() == 2 && "Should load 2 locations");
    std::cout << "  [PASS] Locations inserted and loaded correctly (Count: 2)\n";

    // 2. Roads
    models::Road rd1{"RD-1", "Depot", "Sector 4", 12.5, "Open"};
    assert(db.insertRoad(rd1));
    assert(db.updateRoadStatus("RD-1", "Blocked"));
    auto roads = db.loadAllRoads();
    assert(roads.size() == 1 && roads[0].status == "Blocked");
    std::cout << "  [PASS] Roads inserted and status toggled to Blocked\n";

    // 3. Teams
    models::RescueTeam t1;
    t1.id = "T99";
    t1.name = "Alpha Squad";
    t1.type = "Medical Team";
    t1.capacity = 8;
    t1.currentLocation = "Depot";
    t1.status = "Available";
    t1.equipment = "Trauma Gear";
    assert(db.insertTeam(t1));
    assert(db.updateTeamStatus("T99", "Busy", "R999", "Sector 4", 15.0));
    auto teams = db.loadAllTeams();
    assert(teams.size() == 1 && teams[0].status == "Busy" && teams[0].assignedRequestId == "R999");
    std::cout << "  [PASS] Rescue team inserted, updated to Busy, verified\n";

    // 4. Resources
    models::Resource res1{"RES-1", "Medical Kits", "Medical", 50, 10, "kits"};
    assert(db.insertOrUpdateResource(res1));
    assert(db.deductResource("Medical Kits", 5));
    auto rList = db.loadAllResources();
    assert(rList.size() == 1 && rList[0].quantity == 45);
    assert(db.returnResource("Medical Kits", 5));
    rList = db.loadAllResources();
    assert(rList[0].quantity == 50);
    std::cout << "  [PASS] Resource stock updated, deducted, returned cleanly\n";

    // 5. Reports
    models::Request rep1;
    rep1.id = "R999";
    rep1.personName = "Test Officer";
    rep1.location = "Sector 4";
    rep1.emergencyType = "Earthquake";
    rep1.priority = "High";
    rep1.affectedPeople = 20;
    rep1.immediateRescueCount = 5;
    rep1.requiredResourceType = "Medical Kits";
    rep1.requiredResourceQty = 5;
    rep1.requiredCapability = "Medical Team";
    rep1.description = "Immediate medical triage needed";
    rep1.status = "Pending";
    assert(db.insertReport(rep1));
    assert(db.escalateReport("R999"));
    assert(db.updateReportStatus("R999", "Assigned", "T99"));
    auto reps = db.loadAllReports();
    assert(reps.size() == 1 && reps[0].priority == "Critical" && reps[0].assignedTeamId == "T99");
    std::cout << "  [PASS] Report inserted, escalated to Critical, and assigned\n";

    // 6. Allocations & Operations
    models::Allocation alc1{"ALC-1", "R999", "Test Officer", "Sector 4", "Medical Kits", 5, "2026-10-26 12:00:00", "Deployed"};
    assert(db.insertAllocation(alc1));
    auto alcs = db.loadAllAllocations();
    assert(alcs.size() == 1);
    assert(db.insertOperationRecord("R999", "T99", "Assigned", "Fast route"));
    assert(db.updateOperationRecordStatus("R999", "Completed"));
    std::cout << "  [PASS] Allocation and rescue operation history recorded\n";

    // 7. Operation History
    models::Operation op1;
    op1.id = "1";
    op1.timestamp = "2026-10-26 12:00:00";
    op1.description = "Test operation completed";
    op1.category = "System";
    op1.relatedId = "R999";
    assert(db.insertHistory(op1));
    auto hist = db.loadAllHistory();
    assert(hist.size() == 1);
    std::cout << "  [PASS] Operation history log verified\n";

    // 8. Transactions
    assert(db.beginTransaction());
    models::Resource res2{"RES-2", "Water Rations", "Food", 100, 20, "bottles"};
    db.insertOrUpdateResource(res2);
    assert(db.commitTransaction());
    assert(db.loadAllResources().size() == 2);
    std::cout << "  [PASS] Database transaction BEGIN/COMMIT verified\n";

    std::cout << "============================================================\n";
    std::cout << "  ALL SQLITE DATABASE TESTS PASSED!\n";
    std::cout << "============================================================\n";

    return 0;
}
