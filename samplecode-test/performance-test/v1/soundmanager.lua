SoundManager = Core.class()

function SoundManager:init()
	self.sounds = {
		hit = {sound = Sound.new("sound/hit.wav"), time = 0, delay = 0.05},
	}
end

function SoundManager:play(name)	
	local sound = self.sounds[name]

	local curr = os.timer()
	local prev = sound.time

	if curr - prev > sound.delay then
		sound.sound:play()
		sound.time = curr
	end
end

soundManager = SoundManager.new()
