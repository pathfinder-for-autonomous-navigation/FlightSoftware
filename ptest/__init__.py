import os

def get_pio_asset(asset):
    """
    Searches for a PIO-compiled utility and compiles it
    if it is not available.
    """

    asset_filepath = f".pio/build/{asset}/program" 
    if not os.path.exists(asset_filepath):
        print(f"Compiling PlatformIO target {asset}.")
        os.system(f"pio run -e {asset} > /dev/null")
    return asset_filepath
