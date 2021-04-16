import pylab as plt
import json
import matplotlib.dates as mdates
import mplcursors
from .gpstime import GPSTime
import json
from argparse import ArgumentParser
import cmd, sys

class StateFieldPlotter(object):
    """
    Plotting utility for processing data, both live during the simulation and in
    post-processing.
    """

    def __init__(self, dataList = None):
        self.dataList = dataList
        

        # Clear plot and set plotting ticker parameters
        date_locator = mdates.AutoDateLocator()
        plt.clf()
        plt.gca().xaxis.set_major_formatter(
            mdates.AutoDateFormatter(date_locator))
        plt.gca().xaxis.set_major_locator(date_locator)

    def find_timeseries(self, field, dataList = None):
        if not dataList:
            dataList = self.dataList
        if not dataList:
            print("Database is not available for searching.")
            return None

        suffix = None
        if any(field.endswith(x) for x in ['.u', '.x', '.y', '.z']):
            suffix = field[-1]
            field = field[0:-3]

        field_data = []
        for datapoint in self.dataList:
            if datapoint["field"] == field:
                if suffix is not None:
                    val = datapoint["val"].split(",")[:-1]
                    is_vector = len(val) == 3
                    is_quat = len(val) == 4
                    if not (is_vector or is_quat):
                        print(f"Field {field} does not have any sub-indices.")
                        return None

                    if   suffix == 'w' and is_quat  : idx = 0
                    elif suffix == 'x' and is_quat  : idx = 1
                    elif suffix == 'y' and is_quat  : idx = 2
                    elif suffix == 'z' and is_quat  : idx = 3
                    elif suffix == 'x' and is_vector: idx = 0
                    elif suffix == 'y' and is_vector: idx = 1
                    elif suffix == 'z' and is_vector: idx = 2
                    else:
                        print(f"Invalid index '{suffix}' specified for field {field}.")
                        return None
                    field_data.append((datapoint["time"], val[idx]))
                else:
                    field_data.append((datapoint["time"], datapoint["val"]))

        if len(field_data) == 0:
            print(
                f"Could not find any data for field with name \"{field}\"."
            )
            return None
        else:
            return field_data

    def add_timeseries(self, field, field_data):
        """
        Add time series for a state field named "field" to the plot.
        Expects an array of time-value pairs. The times should be datetime strings.
        """

        # Process times in data
        data_t = [mdates.datestr2num(datapoint[0]) for datapoint in field_data]

        # Process values in data
        if field_data[0][1].count(",") == 2:
            # It's a GPS time
            data_vals = [
                GPSTime(int(datapoint[1])).to_ns() for datapoint in field_data
            ]
            plt.plot(data_t, data_vals, label=field, marker='o', markersize=2)
        elif field_data[0][1].count(",") == 3:
            # It's a vector
            data_vals = [datapoint[1].split(",") for datapoint in field_data]
            data_vals_x = [float(dataval[0]) for dataval in data_vals]
            data_vals_y = [float(dataval[1]) for dataval in data_vals]
            data_vals_z = [float(dataval[2]) for dataval in data_vals]
            plt.plot(data_t, data_vals_x, label=field + ".x", marker='o', markersize=2)
            plt.plot(data_t, data_vals_y, label=field + ".y", marker='o', markersize=2)
            plt.plot(data_t, data_vals_z, label=field + ".z", marker='o', markersize=2)
        elif field_data[0][1].count(",") == 4:
            # It's a quaternion
            data_vals = [datapoint[1].split(",") for datapoint in field_data]
            data_vals_w = [float(dataval[0]) for dataval in data_vals]
            data_vals_x = [float(dataval[1]) for dataval in data_vals]
            data_vals_y = [float(dataval[2]) for dataval in data_vals]
            data_vals_z = [float(dataval[3]) for dataval in data_vals]
            plt.plot(data_t, data_vals_w, label=field + ".u", marker='o', markersize=2)
            plt.plot(data_t, data_vals_x, label=field + ".x", marker='o', markersize=2)
            plt.plot(data_t, data_vals_y, label=field + ".y", marker='o', markersize=2)
            plt.plot(data_t, data_vals_z, label=field + ".z", marker='o', markersize=2)
        else:
            if field_data[0][1] in ["true", "false"]:
                # It's a boolean
                data_vals = [(1 if datapoint[1] == "true" else 0)
                             for datapoint in field_data]
            else:
                try:
                    # It might be an integer
                    data_vals = [int(datapoint[1]) for datapoint in field_data]
                except ValueError:
                    try:
                        # It's a float or double
                        data_vals = [
                            float(datapoint[1]) for datapoint in field_data
                        ]
                    except ValueError:
                        print(f"Field {field} is not of a plottable type.")
                        return False

            plt.plot(data_t, data_vals, label=field, marker='o', markersize=2)

        return True

    def display(self):
        mplcursors.cursor()
        plt.gcf().autofmt_xdate()
        plt.legend()
        plt.show()

class PlotterClient(cmd.Cmd):
    def __init__(self, dataList):
        self.dataList = dataList

        self.intro = "Type \"plot x\" to plot state field \"x\". You can also plot multiple state fields. \n"
        self.prompt = "> "
        super().__init__()

    def do_plot(self, fields):
        """Plots the fields with the given names."""

        fields = fields.split()
        if len(fields) == 0:
            print("Need to specify at least one state field to plot.")
            return

        plotter = StateFieldPlotter(self.dataList)
        for field in fields:
            field_data = plotter.find_timeseries(field, self.dataList)
            if not field_data:
                return
            field_plotted = plotter.add_timeseries(field, field_data)
            if not field_plotted:
                return
        plotter.display()
    
    def do_exit(self, args):
        """Exits the plotter."""
        sys.exit(0)

    def do_quit(self, args):
        """Exits the plotter."""
        sys.exit(0)

def main(args):
    if sys.version_info[0] != 3 or sys.version_info[1] < 6:
        print("Running this script requires Python 3.6 or above.")
        sys.exit(1)

    parser = ArgumentParser("""Allows plotting data after a simulation has ended.""")
    parser.add_argument(
        '-d',
        '--data',
        action='store',
        help=
        '''Location of data telemetry file, relative to the current working directory.''',
        required=True)

    args = parser.parse_args(args)

    
    try:
        fp = open(args.data, "r")
        dataList = json.load(fp)
        fp.close()
    except FileNotFoundError:
        print("Could not find data file. Exiting.")
        sys.exit(1)

    plotter = PlotterClient(dataList)

    try:
        plotter.cmdloop()
    except KeyboardInterrupt:
        # Gracefully exit session
        print("Exiting due to keyboard interrupt.")
        plotter.do_quit(None)
