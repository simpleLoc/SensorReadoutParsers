import io
import typing

from pathlib import Path

import pandas as pd


class YasminMeasurement:
    def __init__(self, ts, mac, rssi):
        self.timestamp = ts
        self.mac = mac
        self.rssi = rssi


class YasminFingerprint:
    def __init__(self, pos: (float, float, float), measurements: typing.List[YasminMeasurement]):
        self.pos = pos
        self.measurements: typing.List[YasminMeasurement] = measurements


class YasminFingerprints:
    def __init__(self, values: typing.List[YasminFingerprint]):
        self.values = values

    def filter_for_mac(self, macFilter: typing.Set[str]): # -> YasminFingerprints:
        result: typing.List[YasminFingerprint] = []

        if isinstance(macFilter, str):
            macFilter = {macFilter}

        for fp in self.values:
            filteredValues = list(filter(lambda x: x.mac in macFilter, fp.measurements))

            if len(filteredValues) > 0:
                result.append(YasminFingerprint(fp.pos, filteredValues))

        return YasminFingerprints(result)

    def to_data_frame(self) -> pd.DataFrame:
        rows = []

        for fp in self.values:
            for m in fp.measurements:
                values = [m.timestamp, m.mac, fp.pos[0], fp.pos[1], fp.pos[2], m.rssi]
                rows.append(values)

        return pd.DataFrame(data=rows, columns=['timestamp', 'mac', 'posX', 'posY', 'posZ', 'rssi'])



class YasminParser:
    def __init__(self, filename: Path):
        self.filename = filename
        self.file: io.FileIO = None
        self.lineNum = 0

    def _readline(self) -> str:
        l = self.file.readline()
        l = l.strip() # remove new line
        self.lineNum += 1
        return l

    def _error(self, message: str):
        raise RuntimeError(f'{message} {self.lineNum}')

    def _parsePos(self) -> (float, float, float):
        line = self._readline()
        posParts = line.split(' ')

        if len(posParts) != 4:
            self._error('invalid pos: line format')
        if posParts[0] != 'pos:':
            self._error('expecting pos: tag')

        posX = float(posParts[1])
        posY = float(posParts[2])
        posZ = float(posParts[3])

        return posX, posY, posZ

    def _parseNum(self) -> int:
        line = self._readline()
        numParts = line.split(' ')

        if len(numParts) != 2:
            self._error('invalid num: line format')
        if numParts[0] != 'num:':
            self._error('expecting pos: tag')

        numValues = int(numParts[1])

        return numValues

    def _parseValue(self) -> YasminMeasurement:
        line = self._readline()
        valueParts = line.split(' ')

        if len(valueParts) != 3:
            self._error('invalid value line format')

        timestamp = int(valueParts[0])
        mac = str(valueParts[1])
        rssi = int(valueParts[2])

        return YasminMeasurement(timestamp, mac, rssi)

    def parse(self) -> YasminFingerprints:
        result: typing.List[YasminFingerprint] = []

        self.file = open(self.filename, 'r')

        while True:
            line = self._readline()

            # EoF?
            if not line:
                break

            if line != '[fingerprint]':
                self._error('expecting [fingerprint] tag')

            # Position
            posX, posY, posZ = self._parsePos()
            # Number of values
            numValues = self._parseNum()

            values: typing.List[YasminMeasurement] = []
            for i in range(numValues):
                m = self._parseValue()
                values.append(m)

            result.append(YasminFingerprint((posX, posY, posZ), values))

        self.file.close()

        return YasminFingerprints(result)


def yasmin_read_file(filename: Path, macFilter=None) -> YasminFingerprints:
    p = YasminParser(filename)
    result = p.parse()

    if macFilter is not None:
        result = result.filter_for_mac(macFilter)

    return result


def yasmin_read_data_frame(filename: Path, macFilter=None) -> pd.DataFrame:
    return yasmin_read_file(filename, macFilter).to_data_frame()
