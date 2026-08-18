import onnxruntime as ort
session = ort.InferenceSession("C:/Users/Byron/Desktop/VoiceClearAI/models/deepfilternet3.onnx")
print("Inputs:")
for i in session.get_inputs():
    print(i.name, i.shape, i.type)
print("Outputs:")
for o in session.get_outputs():
    print(o.name, o.shape, o.type)
