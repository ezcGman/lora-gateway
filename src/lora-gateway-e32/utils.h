std::vector<String> strSplit(char splitChar, String strToSplit) {
  std::vector<String> splittedString;

  while (strToSplit.length() > 0) {
    int splitCharIdx = strToSplit.indexOf(splitChar);

    if (splitCharIdx == -1) {
      splittedString.push_back(strToSplit);
      break;
    }
    else {
      splittedString.push_back(strToSplit.substring(0, splitCharIdx));
      strToSplit = strToSplit.substring(splitCharIdx + 1);
    }
  }

  return splittedString;
}
