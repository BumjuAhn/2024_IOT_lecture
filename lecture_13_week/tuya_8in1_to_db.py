import os
import time
from dotenv import load_dotenv
from tuya_connector import TuyaOpenAPI
import mysql.connector

# 환경 변수 로드
load_dotenv()

# Tuya API 설정
API_ENDPOINT = os.getenv('API_ENDPOINT')
ACCESS_ID = os.getenv('ACCESS_ID')
ACCESS_SECRET = os.getenv('ACCESS_SECRET')
DEVICE_ID = os.getenv('DEVICE_ID_ONLINE_8IN1_TESTER')

# MySQL 연결 설정
def connect_mysql():
    """MySQL 데이터베이스 연결"""
    return mysql.connector.connect(
        host=os.getenv('MYSQL_HOST'),
        port=int(os.getenv('MYSQL_PORT', 3306)),
        user=os.getenv('MYSQL_USER'),
        password=os.getenv('MYSQL_PASSWORD'),
        database=os.getenv('MYSQL_DATABASE')
    )

mysql_connection = connect_mysql()
cursor = mysql_connection.cursor()

# Tuya API 연결
openapi = TuyaOpenAPI(API_ENDPOINT, ACCESS_ID, ACCESS_SECRET)
openapi.connect()

# MySQL 연결 상태 확인
def check_mysql_connection():
    """MySQL 연결 상태 확인 및 재연결"""
    global mysql_connection, cursor
    try:
        mysql_connection.ping(reconnect=True, attempts=3, delay=2)
    except mysql.connector.Error as err:
        print(f"MySQL 연결이 끊어졌습니다. 재연결 시도 중... {err}")
        mysql_connection = connect_mysql()
        cursor = mysql_connection.cursor()

# Tuya API에서 데이터 가져오기
def fetch_tuya_data():
    """Tuya API에서 데이터를 가져옵니다."""
    try:
        response = openapi.get(f"/v2.0/cloud/thing/{DEVICE_ID}/shadow/properties")
        properties = response['result']['properties']
        return {prop['code']: prop['value'] for prop in properties if 'current' in prop['code']}
    except Exception as e:
        print(f"Tuya API 데이터 가져오기 실패: {e}")
        return None

# MySQL에 데이터 저장
def save_to_mysql(result, timestamp):
    """MySQL에 데이터를 저장합니다."""
    check_mysql_connection()  # 연결 상태 확인
    try:
        query = """
            INSERT INTO tuya_8in1 (t, temp_current, ph_current, tds_current, ec_current, salinity_current, pro_current, orp_current, cf_current, rh_current)
            VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s)
        """
        cursor.execute(query, (timestamp, *[result.get(key, 0) for key in [
            "temp_current", "ph_current", "tds_current", "ec_current", 
            "salinity_current", "pro_current", "orp_current", 
            "cf_current", "rh_current"
        ]]))
        mysql_connection.commit()
        print("데이터가 성공적으로 저장되었습니다.")
    except mysql.connector.Error as err:
        print(f"MySQL 저장 오류: {err}")

# 메인 루프
if __name__ == "__main__":
    last_time = time.time()

    try:
        while True:
            # 10초 주기로 데이터 수집 및 저장
            if time.time() - last_time >= 10:
                result = fetch_tuya_data()
                if result:
                    timestamp = int(result.pop('t', time.time() * 1000))  # 타임스탬프 가져오기
                    save_to_mysql(result, timestamp)
                last_time = time.time()

    except KeyboardInterrupt:
        print("프로그램이 중지되었습니다.")

    finally:
        # 자원 해제
        cursor.close()
        mysql_connection.close()
        print("MySQL 연결이 종료되었습니다.")
