import logging
from datetime import datetime, timedelta
from airflow.decorators import dag, task

logger = logging.getLogger(__name__)

default_args = {
    'owner': 'airflow',
    'retries': 1,
    'retry_delay': timedelta(minutes=1),
}



@dag(
    dag_id="print_log_dag",
    is_paused_upon_creation=True,
    start_date=datetime.now(),
    schedule="* * * * *",
    catchup=False,
    tags=["print" , "log"] ,
)
def print_pipeline():

    @task
    def log():
        timestamp = datetime.now().isoformat()
        logger.info(f"Hello, World! Current timestamp: {timestamp}")
        return f"Hello, World! Current timestamp: {timestamp}"

    log()

instance = print_pipeline()