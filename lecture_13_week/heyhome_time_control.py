import os
import time
import json
import requests
from dotenv import load_dotenv

### 환경 변수 로드
load_dotenv()

### JSON 파일에서 토큰 및 장치 ID 로드
def load_token(file_path):
    """
    JSON 파일에서 토큰 및 장치 ID를 로드합니다.
    :param file_path: JSON 파일 경로
    :return: JSON 데이터 딕셔너리
    """
    try:
        with open(file_path, 'r', encoding='utf-8') as file:
            return json.load(file)
    except (FileNotFoundError, json.JSONDecodeError):
        print(f"파일을 찾을 수 없거나 JSON 형식이 잘못되었습니다: {file_path}")
        return None

# 토큰 및 장치 ID 불러오기
token_deviceId = load_token('token_deviceId.json')
if not token_deviceId:
    print("토큰 및 장치 ID를 불러올 수 없습니다. 프로그램을 종료합니다.")
    exit(1)

ACCESS_TOKEN = token_deviceId.get('token')
DEVICE_ID = token_deviceId.get('device_id')

if not ACCESS_TOKEN or not DEVICE_ID:
    print("토큰 또는 장치 ID가 JSON 파일에 누락되었습니다. 프로그램을 종료합니다.")
    exit(1)

### Heyhome Config 로드
BASE_URL = os.getenv('BASE_URL')
if not BASE_URL:
    print("BASE_URL 환경 변수가 설정되지 않았습니다. .env 파일을 확인하세요.")
    exit(1)

# 헤더 설정
HEADERS = {
    "Authorization": f"Bearer {ACCESS_TOKEN}",
    "Content-Type": "application/json",
}

### 장치 제어 함수
def control_power_strip_ports(device_id, states, description):
    """
    멀티탭 포트 상태를 제어하는 함수.
    :param device_id: 제어할 장치의 ID
    :param states: 포트 상태 딕셔너리
    :param description: 단계 설명
    """
    url = f"{BASE_URL}/control/{device_id}"
    payload = {"requirments": states}

    try:
        response = requests.post(url, headers=HEADERS, data=json.dumps(payload))
        if response.status_code == 200:
            print(f"{description}: {states}")
        else:
            print(f"장치 제어 실패 (ID: {device_id}). 상태 코드: {response.status_code}, 응답: {response.text}")
    except requests.RequestException as e:
        print(f"API 요청 중 오류 발생: {e}")

### 주기적 제어 함수
def cycle_control(device_id):
    """
    주기적으로 멀티탭 포트 상태를 제어하는 함수.
    :param device_id: 제어할 장치의 ID
    """
    steps = [
        {"description": "Turning ON fog", "states": {"power1": True, "power2": False, "power3": True}, "delay": 10},
        {"description": "Turning OFF fog", "states": {"power1": False, "power2": False, "power3": True}, "delay": 10},
        {"description": "Turning ON plasma", "states": {"power1": False, "power2": True, "power3": True}, "delay": 10},
        {"description": "Turning OFF plasma", "states": {"power1": False, "power2": False, "power3": True}, "delay": 10},
        {"description": "Turning ON fog and plasma", "states": {"power1": True, "power2": True, "power3": True}, "delay": 10},
        {"description": "Turning OFF fog and plasma", "states": {"power1": False, "power2": False, "power3": True}, "delay": 10},
    ]

    while True:
        for step in steps:
            try:
                control_power_strip_ports(device_id, step["states"], step["description"])
            except Exception as e:
                print(f"단계 실행 중 오류 발생: {step['description']}. 오류: {e}")
            finally:
                time.sleep(step["delay"])  # 각 단계별 대기 시간

### 메인 함수
def main():
    """
    프로그램 진입점. 주기적 제어를 시작합니다.
    """
    try:
        print("주기적 제어 시작...")
        cycle_control(DEVICE_ID)
    except KeyboardInterrupt:
        print("주기적 제어가 수동으로 중지되었습니다.")

### 프로그램 시작
if __name__ == "__main__":
    main()
