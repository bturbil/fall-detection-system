import os
import glob
import sys
import numpy as np
import pandas as pd
import joblib
from scipy.signal import lfilter
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.neural_network import MLPClassifier
from sklearn.pipeline import Pipeline
from sklearn.metrics import classification_report

# ==========================================
# 1. SETTINGS
# ==========================================

DATASET_DIR = "SisFall_dataset" 
MODEL_SAVE_PATH = "mlp_dynamic_only_model.pkl"
WINDOW_SIZE = 300  

# ==========================================
# 2. FEATURE EXTRACTION (EXACT C++ EQUIVALENT)
# ==========================================
def extract_28_features(window_data):
 
    # 1. UNIT CONVERSIONS 
    raw_ax = window_data[:, 0] / 32.0
    raw_ay = window_data[:, 1] / 32.0
    raw_az = window_data[:, 2] / 32.0
    raw_gx = window_data[:, 3] / 14.375
    raw_gy = window_data[:, 4] / 14.375
    raw_gz = window_data[:, 5] / 14.375

    # 2. BİQUAD FILTERING
    b = [0.04613188, 0.09226376, 0.04613188]
    a = [1.0, -1.30728503, 0.49181255]
    
    ax = lfilter(b, a, raw_ax)
    ay = lfilter(b, a, raw_ay)
    az = lfilter(b, a, raw_az)
    gx = lfilter(b, a, raw_gx)
    gy = lfilter(b, a, raw_gy)
    gz = lfilter(b, a, raw_gz)

    features = np.zeros(28)
    curr_window_size = len(ax) 

    # Magnitude Calculations
    acc_mag = np.sqrt(ax**2 + ay**2 + az**2)
    gyro_mag = np.sqrt(gx**2 + gy**2 + gz**2)

    mean_ax = np.mean(ax)
    mean_ay = np.mean(ay)
    mean_az = np.mean(az)

    # Dynamic SMA Sum
    dynamic_sma_sum = np.sum(np.abs(ax - mean_ax) + np.abs(ay - mean_ay) + np.abs(az - mean_az))

    # --- FEATURES ---
    features[0] = np.max(acc_mag)
    features[1] = np.mean(acc_mag)
    features[2] = np.std(acc_mag)
    features[3] = np.max(gyro_mag)
    features[4] = np.mean(gyro_mag)
    features[5] = np.std(gyro_mag)
    
    features[6] = mean_ax
    features[7] = np.max(ax)
    features[8] = np.std(ax)
    features[9] = mean_ay
    features[10] = np.max(ay)
    features[11] = np.std(ay)
    features[12] = mean_az
    features[13] = np.max(az)
    features[14] = np.std(az)
    
    mean_gx = np.mean(gx)
    features[15] = mean_gx
    features[16] = np.max(gx)
    features[17] = np.std(gx)
    mean_gy = np.mean(gy)
    features[18] = mean_gy
    features[19] = np.max(gy)
    features[20] = np.std(gy)
    mean_gz = np.mean(gz)
    features[21] = mean_gz
    features[22] = np.max(gz)
    features[23] = np.std(gz)

    # Feature 24: Tilt Angle 
    tilt_ratio = mean_ax / (features[1] + 0.000001)
    tilt_ratio = np.clip(tilt_ratio, -1.0, 1.0)
    features[24] = np.arccos(tilt_ratio) * (180.0 / np.pi)

    # Feature 25: Dynamic SMA
    features[25] = dynamic_sma_sum / curr_window_size

    # Feature 26-27: Jerk 
    jerk = np.abs(acc_mag[1:] - acc_mag[:-1])
    if len(jerk) > 0:
        features[26] = np.max(jerk)
        features[27] = np.sum(jerk) / (curr_window_size - 1)
    else:
        features[26] = 0
        features[27] = 0

    return features

# ==========================================
# 3. DATASET SCANNING AND LABELING
# ==========================================
print(f"'{DATASET_DIR}' scanning...")

X_list = []
y_list = []


all_files = glob.glob(os.path.join(DATASET_DIR, "**", "*.txt"), recursive=True)

if len(all_files) == 0:
    print(f"ERROR: No .txt files were found in the '{DATASET_DIR}' folder")
    
    sys.exit()
    
print(f" A total of {len(all_files)} .txt files were found. Filtering is starting")

# Labeling map
# D01, D04: Walking | D02: Fast Walking | D03: Running
valid_activities = {
    'D01': 0, 'D04': 0, 
    'D02': 6,            
    'D03': 1             
}

hata_sayisi = 0

for file_path in all_files:
    filename = os.path.basename(file_path)
    activity_code = filename.split('_')[0] 
    
    label = None
    if activity_code.startswith('F'):
        label = 5 
    elif activity_code in valid_activities:
        label = valid_activities[activity_code]
    else:
        continue 
        
    try:
        
        df_file = pd.read_csv(file_path, header=None, sep=r'[,;]', engine='python', on_bad_lines='skip')
        raw_data = df_file.values
        
        
        num_windows = len(raw_data) // WINDOW_SIZE
        
        for w in range(num_windows):
            start_idx = w * WINDOW_SIZE
            end_idx = start_idx + WINDOW_SIZE
            window = raw_data[start_idx:end_idx, :]
            
            
            features = extract_28_features(window)
            
            X_list.append(features)
            y_list.append(label)
            
    except Exception as e:
        if hata_sayisi < 5: 
            print(f"File reading/processing error ({filename}): {e}")
        hata_sayisi += 1

if hata_sayisi > 0:
    print(f"Minor processing errors occurred in {hata_sayisi} files, and those files were skipped.")

if len(X_list) == 0:
    print(" The files were read, but no data windows could be extracted")
    
    sys.exit()

X = np.array(X_list)
y = np.array(y_list)

print(f"Static activity files were removed")
print(f"Total processed dynamic data windows (rows): {len(X)}")

print("\n--- New class distribution ---")
unique, counts = np.unique(y, return_counts=True)
labels_dict = {0: "walking", 1: "running", 5: "fall", 6: "fast walking"}
for u, c in zip(unique, counts):
    print(f"{labels_dict.get(u, f'Sınıf {u}')}: {c} adet pencere")
print("---------------------------\n")

# ==========================================
# 4. MODEL EĞİTİMİ
# ==========================================
X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.2, random_state=42, stratify=y
)

print("Training the model")

pipeline = Pipeline([
    ('scaler', StandardScaler()),
    ('mlp', MLPClassifier(
        hidden_layer_sizes=(64, 32), 
        activation='relu',
        solver='adam',
        alpha=0.001,                 
        batch_size=256,
        learning_rate_init=0.001,
        max_iter=500,
        random_state=42,
        early_stopping=True          
    ))
])

pipeline.fit(X_train, y_train)

# ==========================================
# 5. EVALUATION AND RECORD
# ==========================================
y_pred = pipeline.predict(X_test)

print("\nTrain completed\n")
print("--- Classification report ---")
target_names = [labels_dict.get(i, f"Sınıf {i}") for i in sorted(list(set(y)))]
print(classification_report(y_test, y_pred, target_names=target_names))

joblib.dump(pipeline, MODEL_SAVE_PATH)
print("="*50)
print(f"New model saved: {MODEL_SAVE_PATH}")
print("="*50)
