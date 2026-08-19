
import tkinter as tk
import tkinter.ttk as ttk
import winsound
import psycopg2
import customtkinter as ctk
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure


# ==========================
# Configuration
# ==========================

DB_CONFIG = {
    "host": "localhost",
    "port": 5432,
    "database": "FINAL_TEST",
    "user": "postgres",
    "password": "GAS_SENSOR_123",
}

POLL_INTERVAL_MS = 2000
MAX_HISTORY_POINTS = 30

# ==========================
# ADC ALARM THRESHOLDS
# ==========================

WARNING_THRESHOLD = 750
DANGER_THRESHOLD = 1350


# ==========================
# Dashboard Application
# ==========================

class GasDashboardApp(ctk.CTk):

    def __init__(self):
        super().__init__()

        # Main window setup
        self.title("ESP32 Gas Monitoring Dashboard")
        self.geometry("1100x700")
        self.minsize(1100, 700)
        self.configure(fg_color="#0f172a")
        self.protocol("WM_DELETE_WINDOW", self.on_close)

        # State variables
        self.conn = None
        self.connected = False
        self.timestamp_column = None
        self.last_status = None
        self.blinking = False
        self.blink_state = True
        self.alert_beeped = False

        # Build UI
        self.create_ui()

        # Start refresh
        self.after(300, self.refresh_dashboard)


    # ==========================
    # UI CREATION
    # ==========================

    def create_ui(self):

        self.grid_columnconfigure(0, weight=1)
        self.grid_rowconfigure(1, weight=1)

        # --------------------------
        # Header
        # --------------------------

        title_label = ctk.CTkLabel(
            self,
            text="ESP32 Gas Monitoring Dashboard",
            font=("Segoe UI", 28, "bold"),
            text_color="#f8fafc",
            anchor="center",
            pady=10,
        )

        title_label.grid(
            row=0,
            column=0,
            sticky="ew",
            padx=20,
            pady=(16, 8)
        )


        # --------------------------
        # Main content
        # --------------------------

        main_frame = ctk.CTkFrame(
            self,
            fg_color="#111827",
            corner_radius=20
        )

        main_frame.grid(
            row=1,
            column=0,
            sticky="nsew",
            padx=20,
            pady=(0, 16)
        )

        main_frame.grid_columnconfigure(0, weight=1)
        main_frame.grid_rowconfigure(1, weight=1)


        # ==========================
        # STATISTIC CARDS
        # ==========================

        cards_frame = ctk.CTkFrame(
            main_frame,
            fg_color="#111827"
        )

        cards_frame.grid(
            row=0,
            column=0,
            sticky="ew",
            padx=18,
            pady=18
        )

        cards_frame.grid_columnconfigure(0, weight=1)
        cards_frame.grid_columnconfigure(1, weight=1)
        cards_frame.grid_columnconfigure(2, weight=1)


        self.latest_card = self.create_stat_card(
            cards_frame,
            "Latest ADC",
            "--",
            0,
            0
        )

        self.status_card = self.create_stat_card(
            cards_frame,
            "Status",
            "--",
            1,
            0
        )

        self.total_card = self.create_stat_card(
            cards_frame,
            "Total Readings",
            "--",
            2,
            0
        )


        # ==========================
        # GRAPH + TABLE
        # ==========================

        content_frame = ctk.CTkFrame(
            main_frame,
            fg_color="#111827"
        )

        content_frame.grid(
            row=1,
            column=0,
            sticky="nsew",
            padx=18,
            pady=(0, 18)
        )

        content_frame.grid_columnconfigure(0, weight=1)
        content_frame.grid_rowconfigure(0, weight=1)
        content_frame.grid_rowconfigure(1, weight=1)


        # ==========================
        # GRAPH PANEL
        # ==========================

        graph_panel = ctk.CTkFrame(
            content_frame,
            fg_color="#1f2937",
            corner_radius=18
        )

        graph_panel.grid(
            row=0,
            column=0,
            sticky="nsew",
            padx=0,
            pady=(0, 10)
        )

        graph_panel.grid_columnconfigure(0, weight=1)
        graph_panel.grid_rowconfigure(0, weight=1)


        self.graph_title = ctk.CTkLabel(
            graph_panel,
            text="Live ADC Trend",
            font=("Segoe UI", 18, "bold"),
            text_color="#f8fafc",
            anchor="w",
        )

        self.graph_title.pack(
            anchor="w",
            padx=16,
            pady=(12, 6)
        )


        # Matplotlib graph

        self.figure = Figure(
            figsize=(8, 3.2),
            dpi=100
        )

        self.figure.patch.set_facecolor("#1f2937")

        self.ax = self.figure.add_subplot(111)

        self.ax.set_facecolor("#1f2937")

        self.ax.grid(
            True,
            color="#475569",
            linewidth=0.8,
            alpha=0.5
        )

        self.ax.set_title(
            "Last 30 ADC Readings",
            color="#f8fafc",
            fontsize=10
        )

        self.ax.set_xlabel(
            "Reading Number",
            color="#f8fafc",
            fontsize=8
        )

        self.ax.set_ylabel(
            "ADC Value",
            color="#f8fafc",
            fontsize=8
        )

        self.ax.tick_params(
            colors="#f8fafc",
            labelsize=8
        )

        self.line, = self.ax.plot(
            [],
            [],
            color="#38bdf8",
            linewidth=2.5
        )

        self.canvas = FigureCanvasTkAgg(
            self.figure,
            master=graph_panel
        )

        self.canvas.draw()

        self.canvas.get_tk_widget().pack(
            fill="both",
            expand=True,
            padx=12,
            pady=(0, 12)
        )


        # ==========================
        # TABLE PANEL
        # ==========================

        table_panel = ctk.CTkFrame(
            content_frame,
            fg_color="#1f2937",
            corner_radius=18
        )

        table_panel.grid(
            row=1,
            column=0,
            sticky="nsew"
        )

        table_panel.grid_columnconfigure(0, weight=1)
        table_panel.grid_rowconfigure(0, weight=1)


        table_title = ctk.CTkLabel(
            table_panel,
            text="Recent ADC Readings",
            font=("Segoe UI", 16, "bold"),
            text_color="#f8fafc",
            anchor="w",
        )

        table_title.pack(
            anchor="w",
            padx=16,
            pady=(10, 6)
        )


        self.table_frame = ttk.Frame(
            table_panel
        )

        self.table_frame.pack(
            fill="both",
            expand=True,
            padx=12,
            pady=(0, 12)
        )

        self.table_frame.grid_columnconfigure(
            0,
            weight=1
        )

        self.table_frame.grid_rowconfigure(
            0,
            weight=1
        )


        self.table = ttk.Treeview(
            self.table_frame,
            columns=(
                "id",
                "adc",
                "status",
                "time"
            ),
            show="headings",
            height=8,
        )

        self.table.grid(
            row=0,
            column=0,
            sticky="nsew"
        )


        self.table.heading(
            "id",
            text="ID"
        )

        self.table.heading(
            "adc",
            text="ADC"
        )

        self.table.heading(
            "status",
            text="Status"
        )

        self.table.heading(
            "time",
            text="Time"
        )


        self.table.column(
            "id",
            width=70,
            anchor="center"
        )

        self.table.column(
            "adc",
            width=120,
            anchor="center"
        )

        self.table.column(
            "status",
            width=140,
            anchor="center"
        )

        self.table.column(
            "time",
            width=180,
            anchor="center"
        )


        scrollbar = ttk.Scrollbar(
            self.table_frame,
            orient="vertical",
            command=self.table.yview
        )

        scrollbar.grid(
            row=0,
            column=1,
            sticky="ns"
        )

        self.table.configure(
            yscrollcommand=scrollbar.set
        )


        # ==========================
        # FOOTER
        # ==========================

        self.footer_label = ctk.CTkLabel(
            self,
            text="Connecting to PostgreSQL...",
            font=("Segoe UI", 14, "bold"),
            text_color="#93c5fd",
            pady=6,
        )

        self.footer_label.grid(
            row=2,
            column=0,
            sticky="ew",
            padx=20,
            pady=(0, 10)
        )


    # ==========================
    # STATISTIC CARD
    # ==========================

    def create_stat_card(
        self,
        parent,
        title,
        value,
        col,
        row
    ):

        card = ctk.CTkFrame(
            parent,
            fg_color="#1f2937",
            corner_radius=18
        )

        card.grid(
            row=row,
            column=col,
            padx=8,
            pady=8,
            sticky="nsew"
        )

        card.grid_columnconfigure(
            0,
            weight=1
        )


        title_label = ctk.CTkLabel(
            card,
            text=title,
            font=("Segoe UI", 14, "bold"),
            text_color="#94a3b8",
            anchor="center",
        )

        title_label.grid(
            row=0,
            column=0,
            padx=12,
            pady=(12, 4)
        )


        value_label = ctk.CTkLabel(
            card,
            text=value,
            font=("Segoe UI", 24, "bold"),
            text_color="#f8fafc",
            anchor="center",
        )

        value_label.grid(
            row=1,
            column=0,
            padx=12,
            pady=(0, 12)
        )


        if title == "Latest ADC":
            self.latest_value_label = value_label

        elif title == "Status":
            self.status_value_label = value_label

        elif title == "Total Readings":
            self.total_value_label = value_label


        return card


    # ==========================
    # DATABASE CONNECTION
    # ==========================

    def connect_db(self):

        try:

            conn = psycopg2.connect(
                **DB_CONFIG
            )

            conn.autocommit = False

            self.connected = True

            self.conn = conn

            return conn

        except Exception:

            self.connected = False

            self.conn = None

            return None


    # ==========================
    # FIND TIMESTAMP COLUMN
    # ==========================

    def detect_timestamp_column(self):

        if not self.conn:
            return None

        try:

            with self.conn.cursor() as cur:

                cur.execute(
                    """
                    SELECT column_name
                    FROM information_schema.columns
                    WHERE table_schema = 'public'
                    AND table_name = 'gas_sensor_data'
                    """
                )

                columns = [
                    row[0].lower()
                    for row in cur.fetchall()
                ]


            for candidate in [
                "created_at",
                "timestamp",
                "inserted_at",
                "recorded_at",
                "time_stamp",
                "datetime"
            ]:

                if candidate in columns:
                    return candidate


            return None

        except Exception:

            return None


    # ==========================
    # FETCH DATABASE DATA
    # ==========================

    def fetch_data(self):

        if not self.conn:
            self.connect_db()

        if not self.conn:

            self.connected = False

            return None


        try:

            if self.timestamp_column is None:

                self.timestamp_column = (
                    self.detect_timestamp_column()
                )


            # IMPORTANT:
            # Database column is still called gas_ppm
            # because server1234.py uses that existing
            # database structure.
            #
            # The VALUE stored in this column is now
            # treated as an ADC reading.

            select_columns = [
                "id",
                "gas_ppm",
                "status"
            ]


            if self.timestamp_column:

                select_columns.append(
                    self.timestamp_column
                )


            select_sql = ", ".join(
                select_columns
            )


            with self.conn.cursor() as cur:

                # Total number of readings

                cur.execute(
                    "SELECT COUNT(*) FROM gas_sensor_data"
                )

                total_count = cur.fetchone()[0]


                # Latest reading

                cur.execute(
                    f"""
                    SELECT {select_sql}
                    FROM gas_sensor_data
                    ORDER BY id DESC
                    LIMIT 1
                    """
                )

                latest_row = cur.fetchone()


                # History

                cur.execute(
                    f"""
                    SELECT {select_sql}
                    FROM gas_sensor_data
                    ORDER BY id DESC
                    LIMIT {MAX_HISTORY_POINTS}
                    """
                )

                history_rows = cur.fetchall()


            return (
                latest_row,
                history_rows,
                select_columns,
                total_count
            )


        except Exception:

            self.connected = False


            if self.conn:

                try:
                    self.conn.close()

                except Exception:
                    pass


                self.conn = None


            return None


    # ==========================
    # STATUS CARD
    # ==========================

    def update_status_card(
        self,
        adc_value,
        db_status=None
    ):

        if adc_value is None:

            self.status_value_label.configure(
                text="Database Disconnected"
            )

            self.status_value_label.configure(
                text_color="#f87171"
            )

            self.latest_value_label.configure(
                text="--"
            )

            self.stop_blinking()

            return


        # Convert database value to integer

        try:

            adc_value = int(adc_value)

        except (
            ValueError,
            TypeError
        ):

            self.status_value_label.configure(
                text="INVALID DATA"
            )

            self.status_value_label.configure(
                text_color="#f87171"
            )

            return


        # ==========================
        # ADC STATUS LOGIC
        # ==========================

        if adc_value < WARNING_THRESHOLD:

            status_text = "NORMAL"

            color = "#4ade80"

            self.stop_blinking()


        elif adc_value < DANGER_THRESHOLD:

            status_text = "WARNING"

            color = "#f59e0b"

            self.stop_blinking()


        else:

            status_text = "DANGER ⚠"

            color = "#f87171"

            self.start_blinking()

            self.play_alert_beep()


        # ==========================
        # UPDATE UI
        # ==========================

        self.status_value_label.configure(
            text=status_text
        )

        self.status_value_label.configure(
            text_color=color
        )


        self.latest_value_label.configure(
            text=str(adc_value)
        )

        self.latest_value_label.configure(
            text_color="#f8fafc"
        )


    # ==========================
    # UPDATE GRAPH
    # ==========================

    def update_graph(
        self,
        history_rows
    ):

        if not history_rows:

            self.line.set_data(
                [],
                []
            )

            self.ax.set_xlim(
                0,
                1
            )

            self.ax.set_ylim(
                0,
                4095
            )

        else:

            # row[1] is the ADC value

            values = [
                int(row[1])
                for row in history_rows
            ]

            values = list(
                reversed(values)
            )


            x_vals = list(
                range(
                    1,
                    len(values) + 1
                )
            )


            self.line.set_data(
                x_vals,
                values
            )


            self.ax.set_xlim(
                0.5,
                max(
                    1,
                    len(values) + 0.5
                )
            )


            # ADC is 12-bit:
            # 0 to 4095

            self.ax.set_ylim(
                0,
                4095
            )


        self.ax.relim()

        self.ax.autoscale_view(
            tight=False
        )

        # Keep ADC graph within 0-4095

        self.ax.set_ylim(
            0,
            4095
        )

        self.canvas.draw_idle()


    # ==========================
    # UPDATE TABLE
    # ==========================

    def update_table(
        self,
        history_rows,
        columns
    ):

        for item in self.table.get_children():

            self.table.delete(
                item
            )


        if not history_rows:

            self.table.insert(
                "",
                "end",
                values=(
                    "-",
                    "-",
                    "-",
                    "-"
                )
            )

            return


        for row in history_rows:

            values = [
                row[0],
                row[1],
                row[2]
            ]


            if (
                self.timestamp_column
                and len(row) > 3
            ):

                values.append(
                    str(row[3])
                )

            else:

                values.append("")


            self.table.insert(
                "",
                "end",
                values=values
            )


    # ==========================
    # DATABASE DISCONNECTED
    # ==========================

    def set_disconnected(self):

        self.connected = False


        self.status_value_label.configure(
            text="Database Disconnected"
        )

        self.status_value_label.configure(
            text_color="#f87171"
        )


        self.latest_value_label.configure(
            text="--"
        )


        self.total_value_label.configure(
            text="0"
        )


        self.footer_label.configure(
            text="Disconnected",
            text_color="#f87171"
        )


        self.stop_blinking()

        self.clear_graph_and_table()


    # ==========================
    # CLEAR GRAPH + TABLE
    # ==========================

    def clear_graph_and_table(self):

        for item in self.table.get_children():

            self.table.delete(
                item
            )


        self.table.insert(
            "",
            "end",
            values=(
                "-",
                "-",
                "-",
                "-"
            )
        )


        self.line.set_data(
            [],
            []
        )


        self.ax.set_xlim(
            0,
            1
        )

        self.ax.set_ylim(
            0,
            4095
        )


        self.canvas.draw_idle()


    # ==========================
    # DANGER BLINKING
    # ==========================

    def start_blinking(self):

        if self.blinking:
            return


        self.blinking = True

        self.blink_state = True

        self.blink_status()


    def stop_blinking(self):

        self.blinking = False

        self.alert_beeped = False


    def blink_status(self):

        if not self.blinking:
            return


        if self.blink_state:

            self.status_value_label.configure(
                text_color="#fda4af"
            )

        else:

            self.status_value_label.configure(
                text_color="#7f1d1d"
            )


        self.blink_state = not self.blink_state


        self.after(
            1000,
            self.blink_status
        )


    # ==========================
    # ALERT BEEP
    # ==========================

    def play_alert_beep(self):

        if self.alert_beeped:
            return


        try:

            winsound.Beep(
                1000,
                400
            )

        except Exception:

            pass


        self.alert_beeped = True


    # ==========================
    # REFRESH DASHBOARD
    # ==========================

    def refresh_dashboard(self):

        try:

            result = self.fetch_data()


            if result is None:

                self.set_disconnected()


            else:

                (
                    latest_row,
                    history_rows,
                    columns,
                    total_count
                ) = result


                if latest_row is None:

                    self.connected = False

                    self.set_disconnected()


                else:

                    self.connected = True


                    self.footer_label.configure(
                        text="Connected to PostgreSQL",
                        text_color="#86efac"
                    )


                    # latest_row[1] contains the
                    # value published by ESP32.
                    #
                    # It is ADC, NOT PPM.

                    adc_value = latest_row[1]


                    self.update_status_card(
                        adc_value
                    )


                    self.total_value_label.configure(
                        text=str(total_count)
                    )


                    self.update_graph(
                        history_rows
                    )


                    self.update_table(
                        history_rows,
                        columns
                    )


        except Exception:

            self.set_disconnected()


        self.after(
            POLL_INTERVAL_MS,
            self.refresh_dashboard
        )


    # ==========================
    # SHUTDOWN
    # ==========================

    def on_close(self):

        try:

            if self.conn:

                self.conn.close()

        except Exception:

            pass


        self.destroy()


# ==========================
# ENTRY POINT
# ==========================

if __name__ == "__main__":

    ctk.set_appearance_mode(
        "dark"
    )

    ctk.set_default_color_theme(
        "dark-blue"
    )


    app = GasDashboardApp()

    app.mainloop()

