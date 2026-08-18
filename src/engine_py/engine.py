import argparse
import sys
import numpy as np
import sounddevice as sd
import time
import warnings

warnings.filterwarnings("ignore")

try:
    from df.enhance import enhance, init_df
    import torch
except ImportError as e:
    print(f"Error loading deepfilternet: {e}")
    sys.exit(1)

def find_device(name_substring, is_input):
    devices = sd.query_devices()
    for i, dev in enumerate(devices):
        if name_substring.lower() in dev['name'].lower():
            if is_input and dev['max_input_channels'] > 0:
                return i
            if not is_input and dev['max_output_channels'] > 0:
                return i
    return None

def main():
    parser = argparse.ArgumentParser(description="Voice Clear AI - Python Inference Engine")
    parser.add_argument("--list-devices", action="store_true", help="List audio devices")
    parser.add_argument("--input", type=int, help="Input device ID")
    parser.add_argument("--output", type=int, help="Output device ID")
    args = parser.parse_args()

    if args.list_devices:
        print(sd.query_devices())
        return

    in_dev = args.input
    out_dev = args.output

    if in_dev is None:
        in_dev = find_device("Capture Input", True)
        if in_dev is None:
            # Fallback to default input
            in_dev = sd.default.device[0]
            print(f"Warning: Capture Input not found, using default input: {in_dev}")
        else:
            print(f"Found input device: {in_dev}")

    if out_dev is None:
        out_dev = find_device("CABLE Input", False)
        if out_dev is None:
            out_dev = sd.default.device[1]
            print(f"Warning: VB-Cable not found, using default output: {out_dev}")
        else:
            print(f"Found VB-Cable output: {out_dev}")

    print("Initializing DeepFilterNet model...")
    model, df_state, _ = init_df()
    print("Model loaded. Starting audio stream...")

    sr = df_state.sr()
    hop_size = df_state.hop_size()
    
    def callback(indata, outdata, frames, time_info, status):
        if status:
            print(status, file=sys.stderr)
        
        audio = torch.from_numpy(indata).T.float()
        enhanced = enhance(model, df_state, audio)
        out_np = enhanced.T.detach().cpu().numpy()
        
        if out_np.shape[0] < frames:
            out_np = np.pad(out_np, ((0, frames - out_np.shape[0]), (0, 0)))
        elif out_np.shape[0] > frames:
            out_np = out_np[:frames, :]
            
        # Ensure single channel output even if input has more
        if out_np.shape[1] > 1:
             out_np = out_np[:, 0:1]
             
        outdata[:] = out_np

    try:
        with sd.Stream(device=(in_dev, out_dev),
                       samplerate=sr, blocksize=hop_size * 4,
                       dtype='float32', channels=1,
                       callback=callback):
            print("=========================================")
            print("🎤 Voice Clear AI is ACTIVE")
            print("=========================================")
            print("Press Ctrl+C to stop.")
            while True:
                time.sleep(0.1)
    except KeyboardInterrupt:
        print("\nStopping audio stream...")
    except Exception as e:
        print(f"Stream error: {e}")

if __name__ == "__main__":
    main()
