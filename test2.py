import tkinter as tk
from os import walk


class App(tk.Tk):
    def __init__(self):
        super().__init__()
        #self.geometry("600x300")
        self.attributes("-fullscreen", True)
        self.configure(background='black')
        self.image_names = []
        self.index_tracker = 0
        self.location = 30
        self.canvas = tk.Canvas(self, width=2000, bg='black', highlightthickness=0)
        self.canvas.pack()
        self.canvas.place(x=100, y=600, anchor=tk.NW)
        self.cuttlefimg = tk.PhotoImage(file="./Flip/cuttlef.png")
        for(dirpath, dirnames, filenames) in walk('./Flip/'):
            for name in filenames:
                self.image_names.append(tk.PhotoImage(file="{}{}".format(dirpath, name)))

        #tk.Button(self, text='Start animation!', command=self.start_animation).pack()
        self.start_animation()

    def start_animation(self):
        self.canvas.delete('all')
        if self.index_tracker < len(self.image_names):
            if (self.location % 1600) < 800:
                oscillate_loc = self.location % 800
            else:
                oscillate_loc = 800 - (self.location % 800)
            self.canvas.create_image(oscillate_loc, 75, image=self.cuttlefimg)
            self.location += 1
            self.index_tracker += 1
            self.after(5, self.start_animation)
        else:
            self.index_tracker = 0
            self.start_animation()


if __name__ == "__main__":
    App().mainloop()
