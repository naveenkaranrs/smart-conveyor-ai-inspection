import cv2
import numpy as np
import time
import json
import urllib.request

try:
    import websocket
except ImportError:
    print("Installing websocket-client...")
    import subprocess
    subprocess.check_call(["pip", "install", "websocket-client"])
    import websocket

# ===== CONFIGURATION =====
ESP32_IP = "192.168.137.248"
PHONE_IP = "192.168.137.224"
CAMERA_URL = f"http://{PHONE_IP}:8080/video"

# ===== DETECTION SETTINGS =====
DAMAGE_THRESHOLD = 0.30       # 30% anomaly ratio to count as damaged
CONSECUTIVE_FRAMES = 3        # Need 3 DAMAGED frames in a row to confirm
SCAN_INTERVAL = 0.8           # Scan every 0.8 seconds

print("=" * 50)
print("  AI QUALITY INSPECTION - DAMAGE DETECTION v2.0")
print("=" * 50)
print(f"  Camera : http://{PHONE_IP}:8080")
print(f"  ESP32  : ws://{ESP32_IP}:81/")
print(f"  Threshold : {DAMAGE_THRESHOLD:.0%} | Confirm after {CONSECUTIVE_FRAMES} frames")
print("=" * 50)

# Step 1: Check if phone is reachable
print(f"\nTesting connection to phone ({PHONE_IP})...")
try:
    urllib.request.urlopen(f"http://{PHONE_IP}:8080/", timeout=5)
    print("[OK] Phone camera is reachable!")
except Exception as e:
    print(f"[ERROR] Cannot reach phone camera: {e}")
    print(f"\n  1. Open browser: http://{PHONE_IP}:8080")
    print("  2. Ensure IP Webcam is running")
    print("  3. Same WiFi for phone + laptop")
    import sys
    sys.exit(1)


class PackageInspector:
    """Smart package damage detector using computer vision."""

    def __init__(self):
        self.reference_frame = None
        self.damage_count = 0       # Consecutive damage frame counter
        self.total_scans = 0
        self.total_damaged = 0

    def set_reference(self, frame):
        """Capture a 'good package' reference for comparison."""
        self.reference_frame = self._preprocess(frame)
        print("[REF] Reference frame captured (good package baseline)")

    def _preprocess(self, frame):
        """Convert frame to grayscale and blur for noise reduction."""
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        blurred = cv2.GaussianBlur(gray, (5, 5), 0)
        return blurred

    def detect_damage(self, frame):
        """
        Multi-method damage detection:
        1. Edge irregularity (tears, dents)
        2. Reference comparison (if reference exists)
        3. Texture anomaly detection
        Returns: (result_str, confidence, detail_frame)
        """
        self.total_scans += 1
        gray = self._preprocess(frame)
        h, w = gray.shape
        scores = []

        # --- Method 1: Edge Density Analysis ---
        # Damaged packages have more irregular edges (tears, folds, dents)
        edges = cv2.Canny(gray, 50, 150)
        
        # Focus on center region (where the package is)
        margin_y, margin_x = int(h * 0.15), int(w * 0.15)
        roi_edges = edges[margin_y:h-margin_y, margin_x:w-margin_x]
        
        edge_density = cv2.countNonZero(roi_edges) / roi_edges.size
        # Normal packages: ~5-15% edge density. Damaged: >20%
        edge_score = min(edge_density / 0.25, 1.0)  # Normalize to 0-1
        scores.append(edge_score * 0.4)  # 40% weight

        # --- Method 2: Contour Irregularity ---
        # Find large contours and check if shapes are irregular
        contours, _ = cv2.findContours(roi_edges, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        irregular_count = 0
        for cnt in contours:
            area = cv2.contourArea(cnt)
            if area < 100:  # Skip tiny noise
                continue
            perimeter = cv2.arcLength(cnt, True)
            if perimeter == 0:
                continue
            circularity = 4 * np.pi * area / (perimeter * perimeter)
            # Very irregular shapes (tears) have low circularity
            if circularity < 0.3:
                irregular_count += 1

        irregularity_score = min(irregular_count / 15.0, 1.0)
        scores.append(irregularity_score * 0.3)  # 30% weight

        # --- Method 3: Reference Comparison (if available) ---
        if self.reference_frame is not None:
            ref_resized = cv2.resize(self.reference_frame, (w, h))
            diff = cv2.absdiff(gray, ref_resized)
            _, thresh = cv2.threshold(diff, 40, 255, cv2.THRESH_BINARY)
            diff_ratio = cv2.countNonZero(thresh) / thresh.size
            diff_score = min(diff_ratio / 0.20, 1.0)
            scores.append(diff_score * 0.3)  # 30% weight
        else:
            # Without reference, give more weight to edges
            scores.append(edge_score * 0.15)
            scores.append(irregularity_score * 0.15)

        # --- Final Score ---
        confidence = sum(scores)
        is_damaged = confidence >= DAMAGE_THRESHOLD

        # Update consecutive counter
        if is_damaged:
            self.damage_count += 1
            self.total_damaged += 1
        else:
            self.damage_count = 0  # Reset on good frame

        # Only confirm DAMAGED after N consecutive frames
        confirmed_damage = self.damage_count >= CONSECUTIVE_FRAMES

        # Draw debug info on frame
        detail = frame.copy()
        # Draw ROI rectangle
        cv2.rectangle(detail, (margin_x, margin_y), (w-margin_x, h-margin_y), (255, 255, 0), 2)
        
        # Status bar
        bar_color = (0, 0, 255) if confirmed_damage else ((0, 165, 255) if is_damaged else (0, 255, 0))
        status = "DAMAGED!" if confirmed_damage else ("Checking..." if is_damaged else "GOOD")
        cv2.putText(detail, f"Status: {status}", (10, 35), cv2.FONT_HERSHEY_SIMPLEX, 1.0, bar_color, 3)
        cv2.putText(detail, f"Confidence: {confidence:.1%}", (10, 70), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255,255,255), 2)
        cv2.putText(detail, f"Edge: {edge_score:.1%} | Contour: {irregularity_score:.1%}", 
                    (10, 100), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (200,200,200), 1)
        cv2.putText(detail, f"Streak: {self.damage_count}/{CONSECUTIVE_FRAMES}", 
                    (10, 125), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (200,200,200), 1)
        cv2.putText(detail, "Press Q=quit | R=set reference", (10, detail.shape[0]-15),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (180,180,180), 1)

        result = "DAMAGED" if confirmed_damage else "GOOD"
        return result, confidence, detail


def connect_to_dashboard():
    try:
        ws = websocket.create_connection(f"ws://{ESP32_IP}:81/", timeout=5)
        print("[OK] Connected to ESP32 Dashboard!")
        return ws
    except Exception as e:
        print(f"[WARN] ESP32 not reachable: {e}")
        print("[INFO] Running standalone mode")
        return None


def main():
    print("\nStarting camera feed...")
    cap = cv2.VideoCapture(CAMERA_URL)

    if not cap.isOpened():
        print("[ERROR] Could not open camera!")
        return

    print("[OK] Camera connected!")
    print("[TIP] Press 'R' to capture a GOOD package as reference")
    print("[TIP] Press 'Q' to quit\n")

    ws = connect_to_dashboard()
    inspector = PackageInspector()
    last_update = time.time()
    last_result = "GOOD"
    last_confidence = 0.0
    last_edge = 0.0
    last_contour = 0.0
    conveyor_stopped = False
    good_count = 0  # Track consecutive GOOD frames after a STOP

    while True:
        ret, frame = cap.read()
        if not ret:
            print("Lost camera. Retrying...")
            time.sleep(2)
            cap = cv2.VideoCapture(CAMERA_URL)
            continue

        # Scan at interval
        if time.time() - last_update > SCAN_INTERVAL:
            result, confidence, detail_frame = inspector.detect_damage(frame)
            last_result = result
            last_confidence = confidence
            print(f"[SCAN] {result} (conf: {confidence:.1%}, streak: {inspector.damage_count}/{CONSECUTIVE_FRAMES})")

            # Track good frames after stop
            if conveyor_stopped:
                if result == "GOOD":
                    good_count += 1
                    print(f"[WAIT] Good frame {good_count}/{CONSECUTIVE_FRAMES} before restart...")
                else:
                    good_count = 0  # Reset if bad again

            # Send to ESP32
            if ws:
                try:
                    ws.send(json.dumps({"type": "inspection", "result": result}))
                    
                    # STOP on damage
                    if result == "DAMAGED" and not conveyor_stopped:
                        ws.send(json.dumps({"type": "command", "value": "STOP"}))
                        conveyor_stopped = True
                        good_count = 0
                        print("[ALERT] DAMAGED! CONVEYOR STOPPED!")
                    
                    # AUTO-RESTART after 3 consecutive GOOD frames
                    elif conveyor_stopped and good_count >= CONSECUTIVE_FRAMES:
                        ws.send(json.dumps({"type": "command", "value": "START"}))
                        conveyor_stopped = False
                        good_count = 0
                        print("[AUTO] 3x GOOD confirmed! CONVEYOR RESTARTED!")
                        
                except:
                    print("[WARN] Dashboard disconnected. Retrying...")
                    ws = connect_to_dashboard()

            last_update = time.time()

        # Always draw stats on every frame
        display = frame.copy()
        h, w = display.shape[:2]
        
        # Status bar background
        bar_color = (0, 0, 200) if last_result == "DAMAGED" else (0, 150, 0)
        cv2.rectangle(display, (0, 0), (w, 160), (0, 0, 0), -1)
        cv2.rectangle(display, (0, 0), (w, 160), bar_color, 2)
        
        # Status text
        status_color = (0, 0, 255) if last_result == "DAMAGED" else (0, 255, 0)
        cv2.putText(display, f"Package: {last_result}", (15, 35), cv2.FONT_HERSHEY_SIMPLEX, 1.0, status_color, 3)
        
        # Confidence bar
        cv2.putText(display, f"Accuracy: {last_confidence:.1%}", (15, 70), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255,255,255), 2)
        bar_w = int((w - 30) * min(last_confidence, 1.0))
        cv2.rectangle(display, (15, 80), (w-15, 95), (50, 50, 50), -1)
        conf_bar_color = (0, 0, 255) if last_confidence >= DAMAGE_THRESHOLD else (0, 255, 0)
        cv2.rectangle(display, (15, 80), (15 + bar_w, 95), conf_bar_color, -1)
        
        # Streak and scan info
        cv2.putText(display, f"Streak: {inspector.damage_count}/{CONSECUTIVE_FRAMES} | Scans: {inspector.total_scans}", 
                    (15, 120), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (200, 200, 200), 1)
        
        # CONVEYOR STATUS (always visible)
        if conveyor_stopped:
            cv2.putText(display, f"CONVEYOR: STOPPED (Good: {good_count}/{CONSECUTIVE_FRAMES})", 
                        (15, 148), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 0, 255), 2)
        else:
            cv2.putText(display, "CONVEYOR: RUNNING", (15, 148), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)
        
        cv2.putText(display, "Q=Quit | R=Set Reference", (15, h - 15), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (180,180,180), 1)
        
        # ROI rectangle
        margin_y, margin_x = int(h * 0.15), int(w * 0.15)
        cv2.rectangle(display, (margin_x, max(160, margin_y)), (w-margin_x, h-margin_y), (255, 255, 0), 2)

        cv2.imshow("AI Quality Inspection v2.0", display)

        key = cv2.waitKey(1) & 0xFF
        if key == ord('q'):
            break
        elif key == ord('r'):
            inspector.set_reference(frame)

    cap.release()
    cv2.destroyAllWindows()
    if ws:
        ws.close()
    print(f"\n--- Session Summary ---")
    print(f"Total scans: {inspector.total_scans}")
    print(f"Damage flags: {inspector.total_damaged}")
    print("System stopped.")


if __name__ == "__main__":
    main()
