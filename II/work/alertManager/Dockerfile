FROM python:3.11.14-slim-trixie

WORKDIR /app

COPY alertManager/requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

COPY model/sensor_model.tflite /app/
COPY model/scaler.pkl /app/
COPY alertManager/main.py /app/

CMD ["python", "main.py"]