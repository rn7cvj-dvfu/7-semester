import logging
from datetime import datetime, timedelta
from airflow.sdk import dag, task

logger = logging.getLogger(__name__)

default_args = {
    'owner': 'airflow',
    'retries': 1,
    'retry_delay': timedelta(minutes=1),
}

@dag(
    dag_id='print_log_dag',
    description='Print log every minute',
    schedule='* * * * *',  # Every minute
    start_date=datetime.now(),
    catchup=False,
    tags=['logging', 'test'],
    default_args=default_args,
)
def print_log_dag():
    @task
    def print_message():
        timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        logger.info(f"📋 Log message at {timestamp}")
        return f"Logged at {timestamp}"
    
    print_message()

# Инстанцировать DAG
print_log_dag_instance = print_log_dag()

if __name__ == "__main__":
    print_log_dag()