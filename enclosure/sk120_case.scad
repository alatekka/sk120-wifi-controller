// =====================================================================
//  Alatekka SK120 WiFi case — علبة على تكّة لمزود الطاقة SK120
//  Original design, MIT licence. https://github.com/alatekka/sk120-wifi-controller
//
//  One file, two cases:
//    UNITS = 1  -> one SK120,  Gridfinity 2x2 footprint (83.5 x 83.5 mm)
//    UNITS = 2  -> two SK120s, Gridfinity 4x2 footprint (167.5 x 83.5 mm)
//                  side by side, one shared DC input, one ESP-01 for both
//
//  PART = "body" | "lid" | "assembly"
//
//  Command line:
//    openscad -D UNITS=1 -D 'PART="body"' -o sk120x1_body.stl sk120_case.scad
//
//  MEASURE YOUR SK120 FIRST (calipers) and edit the "SK120" block below.
//  Seller listings disagree by a millimetre or two.
// =====================================================================

UNITS = 1;            // 1 or 2
PART  = "assembly";   // "body", "lid", "assembly"

/* ---------- SK120 (measure yours) ---------- */
SK_BEZEL_W = 79.0;    // front bezel width
SK_BEZEL_H = 43.0;    // front bezel height
SK_BODY_W  = 71.3;    // body behind the bezel (panel opening)
SK_BODY_H  = 39.0;
SK_DEPTH   = 62.0;    // body + rear fan, behind the panel
FIT        = 0.4;     // clearance added to the panel opening

/* ---------- parts on the panels ---------- */
POST_D     = 6.5;     // 4 mm binding post panel hole (M6 thread)
POST_PITCH = 19.05;   // standard 3/4" banana spacing
DC_D       = 11.0;    // 5.5x2.1 panel jack (DC-022). Use 8.0 for DC-099
USBC_W     = 9.4;     // USB-C PD trigger board receptacle
USBC_H     = 3.6;
ESP_W      = 25.0;    // ESP-01 / ESP-01S pcb 24.8 x 14.3
ESP_L      = 14.6;
PCB_T      = 1.7;

/* ---------- case ---------- */
GF      = 42;         // Gridfinity pitch
CELLS_X = UNITS == 2 ? 4 : 2;
CELLS_Y = 2;
W   = GF * CELLS_X - 0.5;   // 83.5 / 167.5
D   = GF * CELLS_Y - 0.5;   // 83.5
H   = 74;                   // wall height (lid adds 2.4)
WALL  = 2.4;
FLOOR = 1.6;
R     = 3.75;               // outer corner radius (Gridfinity)
BASE  = 4.75;               // Gridfinity foot height
LID_T = 2.4;
LIP_H = 3.0;
LIP_T = 1.6;
SCREW_D = 2.6;              // M3 self-tapping into the posts
FONT_AR = "Arial:style=Bold";
FONT_EN = "Arial:style=Bold";

$fn = 48;
eps = 0.01;

FLOOR_Z = BASE + FLOOR;                       // inside floor
ZC      = H - 3 - SK_BEZEL_H / 2;             // panel centre height
BODY_Z0 = ZC - SK_BODY_H / 2;                 // bottom of SK120 body
POST_Z  = (FLOOR_Z + (ZC - SK_BEZEL_H / 2)) / 2 + 1;
ESP_Y   = 40;                                 // ESP-01 cradle, behind the binding posts

// x centre of each SK120 on the front
function unit_x(i) = UNITS == 1 ? W / 2 : W / 4 + (i == 0 ? 0.6 : -0.6) + i * W / 2;

/* ---------------- helpers ---------------- */
module rsq(sx, sy, r, h) {
  linear_extrude(h) offset(r) square([max(sx - 2*r, eps), max(sy - 2*r, eps)], center = true);
}
module rbox(sx, sy, h, r) { translate([sx/2, sy/2, 0]) rsq(sx, sy, r, h); }
module slot(l, w, h) {        // rounded slot along X, cutting along Y
  hull() for (s = [-1, 1]) translate([s * (l - w) / 2, 0, 0]) rotate([90, 0, 0]) cylinder(d = w, h = h, center = true);
}

/* ---------------- Gridfinity feet (public Gridfinity spec) ---------------- */
module gf_foot() {
  difference() {
    hull() {
      rsq(35.6, 35.6, 0.8, eps);
      translate([0, 0, 0.8])  rsq(37.2, 37.2, 1.6, eps);
      translate([0, 0, 2.6])  rsq(37.2, 37.2, 1.6, eps);
      translate([0, 0, 4.75 - eps]) rsq(41.5, 41.5, 3.75, eps);
    }
    // 6 x 2 mm magnets
    for (mx = [-13, 13], my = [-13, 13]) translate([mx, my, -eps]) cylinder(d = 6.5, h = 2.4);
  }
}
module gf_base() {
  for (i = [0 : CELLS_X - 1], j = [0 : CELLS_Y - 1])
    translate([20.75 + i * GF, 20.75 + j * GF, 0]) gf_foot();
}

/* ---------------- body ---------------- */
module rear_posts(h_extra = 0, d = 7) {
  for (px = [WALL + 4.2, W - WALL - 4.2])
    translate([px, D - WALL - 4.2, 0]) children();
}

module body() {
  difference() {
    union() {
      gf_base();
      translate([0, 0, BASE - eps]) rbox(W, D, H - BASE + eps, R);
    }
    // hollow
    translate([WALL, WALL, FLOOR_Z]) rbox(W - 2*WALL, D - 2*WALL, H, R - WALL > 0.5 ? R - WALL : 0.5);

    // ---- front: SK120 openings, binding posts, labels ----
    for (u = [0 : UNITS - 1]) {
      x = unit_x(u);
      translate([x - (SK_BODY_W + FIT)/2, -eps, ZC - (SK_BODY_H + FIT)/2])
        cube([SK_BODY_W + FIT, WALL + 2*eps, SK_BODY_H + FIT]);
      // shallow seat so the bezel sits flush and square
      translate([x - (SK_BEZEL_W + 0.6)/2, -eps, ZC - (SK_BEZEL_H + 0.6)/2])
        cube([SK_BEZEL_W + 0.6, 0.6, SK_BEZEL_H + 0.6]);
      for (s = [-1, 1]) translate([x + s * POST_PITCH/2, WALL + 1, POST_Z]) rotate([90, 0, 0]) cylinder(d = POST_D, h = WALL + 3);
      // + / − engraved beside the posts
      translate([x - POST_PITCH/2 - 9, 0.5, POST_Z]) rotate([90, 0, 0]) linear_extrude(1)
        text("+", size = 6, font = FONT_EN, halign = "center", valign = "center");
      translate([x + POST_PITCH/2 + 9, 0.5, POST_Z]) rotate([90, 0, 0]) linear_extrude(1)
        text("−", size = 6, font = FONT_EN, halign = "center", valign = "center");
      if (UNITS == 2)
        translate([x, 0.5, FLOOR_Z + 1.6]) rotate([90, 0, 0]) linear_extrude(1)
          text(u == 0 ? "CH1" : "CH2", size = 3.2, font = FONT_EN, halign = "center", valign = "bottom");
    }

    // ---- rear: DC jack, USB-C, fan exhaust per unit ----
    translate([WALL + 13, D + eps, FLOOR_Z + 12]) rotate([90, 0, 0]) cylinder(d = DC_D, h = WALL + 1);
    translate([W - WALL - 16, D - WALL/2, FLOOR_Z + 6.2])
      hull() for (s = [-1, 1]) translate([s * (USBC_W - USBC_H)/2, 0, 0]) rotate([90, 0, 0]) cylinder(d = USBC_H, h = WALL + 1, center = true);
    for (u = [0 : UNITS - 1]) for (k = [0 : 6])
      translate([unit_x(u), D - WALL/2, ZC - 15 + k * 5]) slot(34, 2.6, WALL + 1);

    // ---- side vents (lower, intake) ----
    for (sx = [-eps, W - WALL - eps], k = [0 : 5])
      translate([sx + WALL/2, 30 + k * 7, FLOOR_Z + 8]) rotate([0, 0, 90])
        hull() for (z = [0, 14]) translate([0, 0, z]) rotate([90, 0, 0]) cylinder(d = 2.6, h = WALL + 1, center = true);

    // ---- base logo (engraved into the floor, read through the bottom) ----
  }

  // ---- inside: rails under each SK120 ----
  for (u = [0 : UNITS - 1]) for (s = [-1, 1])
    translate([unit_x(u) + s * (SK_BODY_W/2 - 6) - 2, WALL, FLOOR_Z - eps])
      cube([4, min(SK_DEPTH - 8, D - 2*WALL - 6), BODY_Z0 - FLOOR_Z - 0.3]);
  // stop at the back of each SK120 so it cannot slide in
  for (u = [0 : UNITS - 1])
    translate([unit_x(u) - 12, WALL + SK_DEPTH + 1, FLOOR_Z - eps]) cube([24, 2, BODY_Z0 - FLOOR_Z + 6]);

  // ---- ESP-01 cradle on the floor, under the (first) SK120 ----
  // the pcb slides in from the back between two slotted rails, header pins up
  translate([unit_x(0), ESP_Y, FLOOR_Z - eps]) {
    for (s = [-1, 1]) translate([s > 0 ? ESP_W / 2 + 0.3 : -ESP_W / 2 - 0.3 - 2.4, -ESP_L / 2 - 1, 0])
      difference() {
        cube([2.4, ESP_L + 3, 6]);
        translate([s > 0 ? -eps : 2.4 - 1.0, -eps, 3]) cube([1.0 + eps, ESP_L + 4, PCB_T + 0.3]);
      }
    translate([-ESP_W / 2 - 2.7, -ESP_L / 2 - 3, 0]) cube([ESP_W + 5.4, 2, 6]);   // front stop
  }

  // ---- USB-C PD board pocket: two guides on the floor ----
  for (s = [-1, 1]) translate([W - WALL - 16 + s * 9 - 1, D - WALL - 22, FLOOR_Z - eps]) cube([2, 22, 3.2]);

  // ---- rear screw posts for the lid, with 45° support cone ----
  rear_posts() difference() {
    union() {
      translate([0, 0, H - 14]) cylinder(d = 7, h = 14);
      translate([0, 0, H - 21]) cylinder(d1 = 0.8, d2 = 7, h = 7);
    }
    translate([0, 0, H - 12]) cylinder(d = SCREW_D, h = 13);
  }
}

/* ---------------- lid ---------------- */
module lid() {
  difference() {
    union() {
      rbox(W, D, LID_T, R);
      // locating lip that drops inside the walls
      translate([0, 0, -LIP_H]) difference() {
        translate([WALL + 0.25, WALL + 0.25, 0]) rbox(W - 2*WALL - 0.5, D - 2*WALL - 0.5, LIP_H + eps, 0.6);
        translate([WALL + 0.25 + LIP_T, WALL + 0.25 + LIP_T, -eps]) rbox(W - 2*WALL - 0.5 - 2*LIP_T, D - 2*WALL - 0.5 - 2*LIP_T, LIP_H + 1, 0.4);
      }
    }
    // clear the screw posts and countersink the screws
    rear_posts() {
      translate([0, 0, -LIP_H - 1]) cylinder(d = 8, h = LIP_H + 1);
      translate([0, 0, -1]) cylinder(d = 3.4, h = LID_T + 2);
      translate([0, 0, LID_T - 1.7]) cylinder(d1 = 3.4, d2 = 6.6, h = 1.7 + eps);
    }
    // exhaust slots over the back half
    for (u = [0 : UNITS - 1]) for (k = [0 : 4])
      translate([unit_x(u), D - 34 + k * 5.5, -1]) hull() for (s = [-1, 1]) translate([s * 14, 0, 0]) cylinder(d = 2.6, h = LID_T + 2);
    // brand, engraved
    translate([W / 2, 25, LID_T - 0.6]) linear_extrude(1)
      text("على تكّة", size = UNITS == 2 ? 9 : 7.5, font = FONT_AR, halign = "center", valign = "center", direction = "rtl", language = "ar", script = "arabic");
    translate([W / 2, 13.5, LID_T - 0.6]) linear_extrude(1)
      text(UNITS == 2 ? "SK120 x2  WiFi" : "SK120  WiFi", size = 3.6, font = FONT_EN, halign = "center", valign = "center");
  }
}

/* ---------------- output ---------------- */
if (PART == "body") body();
else if (PART == "lid") translate([0, 0, LID_T]) mirror([0, 0, 1]) lid();   // print top-down
else {
  color("#1b2230") body();
  color("#00b8d4") translate([0, 0, H + 0.2]) lid();
  // simplified SK120 fronts, for the renders only
  for (u = [0 : UNITS - 1]) translate([unit_x(u), 0, ZC]) {
    color("#2b313b") translate([-SK_BEZEL_W/2, -4, -SK_BEZEL_H/2]) cube([SK_BEZEL_W, 4, SK_BEZEL_H]);
    color("#0c1a24") translate([-SK_BEZEL_W/2 + 5, -4.4, -SK_BEZEL_H/2 + 7]) cube([46, 0.5, 30]);       // screen
    color("#22d3ee") translate([-SK_BEZEL_W/2 + 9, -4.6, 4]) cube([22, 0.3, 9]);                         // reading
    color("#facc15") translate([-SK_BEZEL_W/2 + 9, -4.6, -9]) cube([16, 0.3, 7]);
    color("#9aa4b2") translate([SK_BEZEL_W/2 - 15, -4, 3]) rotate([90, 0, 0]) cylinder(d = 16, h = 7);  // knob
    for (k = [0 : 2]) color("#4b5563") translate([SK_BEZEL_W/2 - 24 + k * 9, -4.5, -15]) cube([6, 0.5, 4]);  // keys
  }
}
