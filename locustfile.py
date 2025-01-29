from locust import HttpUser, task, between

class WebsiteUser(HttpUser):
    wait_time = between(1, 2)

    @task
    def get_empty_html(self):
        headers = {
            "User-Agent": "locust-load-test",
            "Accept": "text/html",
        }
        self.client.get("http://localhost:8080/empty.html", headers=headers)

    # def on_start(self):
    #     print("Starting Locust test...")